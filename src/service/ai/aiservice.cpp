#include "service/ai/aiservice.h"

#include "network/openaiclient.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

namespace
{
void writeAiLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | " << message << '\n';
}

QString defaultSystemPrompt()
{
    return "You are an AI programming assistant for algorithm problems. "
           "Answer in concise Chinese when appropriate. "
           "Use the provided problem statement, current language, source code, test input, and test output. "
           "When the user asks to test, run, validate, or check the program, prefer calling run_test. "
           "Use the current test input unless the user explicitly asks to change it. "
           "You may call tools to inspect or modify the workspace.";
}

QString translationSystemPrompt()
{
    return "Translate only the problem description, input, output, and hint into concise Chinese. "
           "Preserve line breaks. Do not translate code, variable names, URLs, or sample IO. "
           "Return strict JSON with keys description, input, output, and hint.";
}

QJsonObject textContentItem(const QString &text)
{
    return QJsonObject{{"type", "input_text"}, {"text", text}};
}

QJsonObject unwrapResponseObject(const QJsonObject &object)
{
    const QJsonObject nestedResponse = object.value("response").toObject();
    if (!nestedResponse.isEmpty() && nestedResponse.value("object").toString() == "response") {
        return nestedResponse;
    }
    return object;
}
}

AiService::AiService(OpenAiClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(m_client, &OpenAiClient::responseDelta, this, [this](const QString &delta) {
        m_accumulatedResponse += delta;
        writeAiLog(QString("AiService: responseDelta len=%1").arg(QString::number(delta.size())));
        emit responseDelta(delta);
    });
    connect(m_client,
            &OpenAiClient::responseCompleted,
            this,
            [this](const QJsonObject &response) {
                writeAiLog(QString("AiService: responseCompleted keys=%1")
                               .arg(QStringList(response.keys()).join(",")));
                handleCompletedResponse(response);
            });
    connect(m_client, &OpenAiClient::requestFailed, this, [this](const QString &message) {
        m_thinking = false;
        emit thinkingChanged(false);
        writeAiLog(QString("AiService: requestFailed %1")
                       .arg(message.left(200).replace('\n', ' ')));
        emit failed(message);
    });
}

void AiService::setConfig(const OpenAiConfig &config)
{
    m_config = config;
}

void AiService::setToolContext(const AiToolContext &context)
{
    m_toolContext = context;
}

OpenAiConfig AiService::config() const
{
    return m_config;
}

void AiService::ask(const QString &question,
                    const QString &problemDetail,
                    const QString &sourceCode,
                    const QString &testInput,
                    const QString &testOutput)
{
    if (m_client == nullptr) {
        emit failed("AI client is not available.");
        return;
    }
    if (m_config.apiKey.trimmed().isEmpty()) {
        emit failed("API key is required.");
        return;
    }
    if (question.trimmed().isEmpty()) {
        emit failed("AI question is required.");
        return;
    }

    m_question = question.trimmed();
    m_problemDetail = problemDetail;
    m_sourceCode = sourceCode;
    m_testInput = testInput;
    m_testOutput = testOutput;
    m_systemPrompt = m_config.systemPrompt.trimmed().isEmpty()
                         ? defaultSystemPrompt()
                         : m_config.systemPrompt;

    m_inputItems = QJsonArray{makeMessageItem(
        "user",
        buildUserPrompt(m_question,
                        m_problemDetail,
                        m_sourceCode,
                        m_testInput,
                        m_testOutput))};
    m_accumulatedResponse.clear();
    m_currentResponseText.clear();
    m_debugLog.clear();
    m_pendingToolCallId.clear();
    m_pendingToolName.clear();
    m_translationMode = false;
    m_responseNonStreamFallbackTried = false;
    m_chatCompletionFallbackTried = false;
    m_thinking = true;
    writeAiLog(QString("AiService::ask begin question=%1").arg(m_question.left(80)));
    emit thinkingChanged(true);
    sendRequest();
}

void AiService::translateProblem(const QString &description,
                                 const QString &inputSpec,
                                 const QString &outputSpec,
                                 const QString &hint)
{
    if (m_client == nullptr) {
        emit failed("AI client is not available.");
        return;
    }
    if (m_config.apiKey.trimmed().isEmpty()) {
        emit failed("API key is required.");
        return;
    }
    if (description.trimmed().isEmpty() && inputSpec.trimmed().isEmpty()
        && outputSpec.trimmed().isEmpty() && hint.trimmed().isEmpty()) {
        emit failed("Nothing to translate.");
        return;
    }

    m_question.clear();
    m_problemDetail.clear();
    m_sourceCode.clear();
    m_testInput.clear();
    m_testOutput.clear();
    m_systemPrompt = translationSystemPrompt();
    m_inputItems = QJsonArray{makeMessageItem(
        "user",
        buildProblemTranslationPrompt(description, inputSpec, outputSpec, hint))};
    m_accumulatedResponse.clear();
    m_currentResponseText.clear();
    m_debugLog.clear();
    m_pendingToolCallId.clear();
    m_pendingToolName.clear();
    m_translationMode = true;
    m_translationChatFallbackTried = false;
    m_responseNonStreamFallbackTried = false;
    m_chatCompletionFallbackTried = false;
    m_thinking = true;
    writeAiLog("AiService::translateProblem begin");
    emit thinkingChanged(true);
    sendRequest();
}

void AiService::completeToolCall(const QString &callId, const QString &outputText)
{
    if (callId.isEmpty()) {
        return;
    }

    const QString completedToolName = m_pendingToolName;
    writeAiLog(QString("AiService::completeToolCall tool=%1 callId=%2 outputLen=%3")
                   .arg(completedToolName,
                        callId,
                        QString::number(outputText.size())));
    if (completedToolName == "run_test") {
        m_testOutput = outputText;
    }

    m_inputItems = QJsonArray{makeMessageItem(
        "user",
        buildToolResultPrompt(completedToolName,
                              outputText,
                              m_problemDetail,
                              m_sourceCode,
                              m_testInput,
                              m_testOutput))};
    m_pendingToolCallId.clear();
    m_pendingToolName.clear();
    sendRequest();
}

void AiService::failToolCall(const QString &callId, const QString &message)
{
    if (callId.isEmpty()) {
        return;
    }

    const QString failedToolName = m_pendingToolName;
    writeAiLog(QString("AiService::failToolCall tool=%1 callId=%2 message=%3")
                   .arg(failedToolName,
                        callId,
                        message.left(160).replace('\n', ' ')));
    m_inputItems = QJsonArray{makeMessageItem(
        "user",
        buildToolResultPrompt(failedToolName,
                              QString("Tool failed.\n%1").arg(message),
                              m_problemDetail,
                              m_sourceCode,
                              m_testInput,
                              m_testOutput))};
    m_pendingToolCallId.clear();
    m_pendingToolName.clear();
    sendRequest();
}

QString AiService::buildUserPrompt(const QString &question,
                                   const QString &problemDetail,
                                   const QString &sourceCode,
                                   const QString &testInput,
                                   const QString &testOutput) const
{
    return QString(
               "User Question\n%1\n\n"
               "Problem Detail\n%2\n\n"
               "Current Language\n%3\n\n"
               "Source Code\n%4\n\n"
               "Test Input\n%5\n\n"
               "Last Test Output\n%6")
        .arg(question,
             problemDetail.isEmpty() ? "<none>" : problemDetail,
             m_toolContext.getCurrentLanguage ? m_toolContext.getCurrentLanguage() : "<none>",
             sourceCode.isEmpty() ? "<none>" : sourceCode,
             testInput.isEmpty() ? "<none>" : testInput,
             testOutput.isEmpty() ? "<none>" : testOutput);
}

QString AiService::buildProblemTranslationPrompt(const QString &description,
                                                 const QString &inputSpec,
                                                 const QString &outputSpec,
                                                 const QString &hint) const
{
    return QString(
               "Description\n%1\n\n"
               "Input\n%2\n\n"
               "Output\n%3\n\n"
               "Hint\n%4\n\n"
               "Return JSON only.")
        .arg(description.trimmed().isEmpty() ? "<none>" : description,
             inputSpec.trimmed().isEmpty() ? "<none>" : inputSpec,
             outputSpec.trimmed().isEmpty() ? "<none>" : outputSpec,
             hint.trimmed().isEmpty() ? "<none>" : hint);
}

QString AiService::buildToolResultPrompt(const QString &toolName,
                                         const QString &toolOutput,
                                         const QString &problemDetail,
                                         const QString &sourceCode,
                                         const QString &testInput,
                                         const QString &testOutput) const
{
    return QString(
               "Continue helping with the same task.\n\n"
               "Tool Executed\n%1\n\n"
               "Tool Result\n%2\n\n"
               "Problem Detail\n%3\n\n"
               "Current Language\n%4\n\n"
               "Source Code\n%5\n\n"
               "Test Input\n%6\n\n"
               "Last Test Output\n%7")
        .arg(toolName.isEmpty() ? "<unknown>" : toolName,
             toolOutput.isEmpty() ? "<none>" : toolOutput,
             problemDetail.isEmpty() ? "<none>" : problemDetail,
             m_toolContext.getCurrentLanguage ? m_toolContext.getCurrentLanguage() : "<none>",
             sourceCode.isEmpty() ? "<none>" : sourceCode,
             testInput.isEmpty() ? "<none>" : testInput,
             testOutput.isEmpty() ? "<none>" : testOutput);
}

QJsonObject AiService::buildRequestPayload(const QString &question,
                                           const QString &problemDetail,
                                           const QString &sourceCode,
                                           const QString &testInput,
                                           const QString &testOutput) const
{
    Q_UNUSED(question);
    Q_UNUSED(problemDetail);
    Q_UNUSED(sourceCode);
    Q_UNUSED(testInput);
    Q_UNUSED(testOutput);

    QJsonObject payload{
        {"model", m_config.model},
        {"instructions", m_systemPrompt},
        {"input", m_inputItems},
    };

    if (!m_translationMode) {
        payload.insert("tools", buildTools());
    }
    return payload;
}

QJsonArray AiService::buildTools() const
{
    auto functionTool = [](const QString &name,
                           const QString &description,
                           const QJsonObject &parameters) {
        return QJsonObject{
            {"type", "function"},
            {"name", name},
            {"description", description},
            {"parameters", parameters},
        };
    };

    const QJsonObject noArgs{
        {"type", "object"},
        {"properties", QJsonObject{}},
        {"required", QJsonArray{}},
        {"additionalProperties", false},
    };
    const QJsonObject textArg{
        {"type", "object"},
        {"properties",
         QJsonObject{{"text", QJsonObject{{"type", "string"},
                                            {"description", "Replacement text"}}}}},
        {"required", QJsonArray{"text"}},
        {"additionalProperties", false},
    };
    const QJsonObject stdinArg{
        {"type", "object"},
        {"properties",
         QJsonObject{{"stdin", QJsonObject{{"type", "string"},
                                            {"description", "Optional test input override for run_test. Leave empty to use the current test input."}}}}},
        {"required", QJsonArray{}},
        {"additionalProperties", false},
    };

    return QJsonArray{
        functionTool("get_current_problem", "Read the current problem statement.", noArgs),
        functionTool("get_current_language", "Read the current selected submit language.", noArgs),
        functionTool("get_current_code", "Read the current source code.", noArgs),
        functionTool("get_test_input", "Read the current test input.", noArgs),
        functionTool("get_test_output", "Read the current test output.", noArgs),
        functionTool("replace_code", "Replace the source code editor content.", textArg),
        functionTool("replace_input", "Replace the test input editor content.", textArg),
        functionTool("run_test", "Run the current code. If stdin is omitted, use the current test input.", stdinArg),
        functionTool("submit_code", "Trigger the current solution submission.", noArgs),
    };
}

QJsonObject AiService::makeMessageItem(const QString &role, const QString &text) const
{
    return QJsonObject{
        {"type", "message"},
        {"role", role},
        {"content", QJsonArray{textContentItem(text)}},
    };
}

void AiService::sendRequest()
{
    if (m_client == nullptr) {
        emit failed("AI client is not available.");
        return;
    }
    if (m_pendingToolCallId.isEmpty()) {
        writeAiLog(QString("AiService::sendRequest translationMode=%1 inputItems=%2")
                       .arg(m_translationMode ? "true" : "false",
                            QString::number(m_inputItems.size())));
        const QJsonObject payload = buildRequestPayload(m_question,
                                                        m_problemDetail,
                                                        m_sourceCode,
                                                        m_testInput,
                                                        m_testOutput);
        QString requestText;
        if (!m_debugLog.isEmpty()) {
            m_debugLog += "\n\n";
        }
        if (m_translationMode) {
            QJsonArray messages;
            messages.append(QJsonObject{
                {"role", "system"},
                {"content", m_systemPrompt},
            });
            if (!m_inputItems.isEmpty()) {
                const QJsonObject item = m_inputItems.first().toObject();
                const QJsonArray contentArray = item.value("content").toArray();
                QString userText;
                for (const QJsonValue &contentValue : contentArray) {
                    const QJsonObject contentObject = contentValue.toObject();
                    if (contentObject.value("type").toString() == "input_text") {
                        userText += contentObject.value("text").toString();
                    }
                }
                messages.append(QJsonObject{
                    {"role", "user"},
                    {"content", userText},
                });
            }

            QJsonObject chatPayload{
                {"model", m_config.model},
                {"messages", messages},
                {"stream", false},
            };
            requestText =
                QString("OpenAI Request\n"
                        "POST %1\n\n"
                        "Payload:\n%2")
                    .arg(m_config.baseUrl.toString().endsWith('/')
                             ? m_config.baseUrl.toString() + "chat/completions"
                             : m_config.baseUrl.toString() + "/chat/completions",
                         QString::fromUtf8(
                             QJsonDocument(chatPayload).toJson(QJsonDocument::Indented)));
            m_debugLog += requestText;
            m_translationChatFallbackTried = true;
            writeAiLog("AiService::sendRequest using chat/completions fallback");
            m_client->createChatCompletion(m_config.apiKey.trimmed(), chatPayload);
        } else {
            const QJsonObject debugPayload = QJsonObject(payload);
            requestText =
                QString("OpenAI Request\n"
                        "POST %1\n\n"
                        "Payload:\n%2")
                    .arg(m_config.baseUrl.toString().endsWith('/')
                             ? m_config.baseUrl.toString() + "responses"
                             : m_config.baseUrl.toString() + "/responses",
                         QString::fromUtf8(
                             QJsonDocument(debugPayload).toJson(QJsonDocument::Indented)));
            m_debugLog += requestText;
            writeAiLog("AiService::sendRequest using responses stream");
            m_client->createResponseStream(m_config.apiKey.trimmed(), payload);
        }
    }
}

void AiService::sendResponseFallback()
{
    if (m_client == nullptr) {
        emit failed("AI client is not available.");
        return;
    }

    const QJsonObject payload = buildRequestPayload(m_question,
                                                    m_problemDetail,
                                                    m_sourceCode,
                                                    m_testInput,
                                                    m_testOutput);
    const QString requestText =
        QString("OpenAI Request\n"
                "POST %1\n\n"
                "Payload:\n%2")
            .arg(m_config.baseUrl.toString().endsWith('/')
                     ? m_config.baseUrl.toString() + "responses"
                     : m_config.baseUrl.toString() + "/responses",
                 QString::fromUtf8(
                     QJsonDocument(payload).toJson(QJsonDocument::Indented)));
    if (!m_debugLog.isEmpty()) {
        m_debugLog += "\n\n";
    }
    m_debugLog += requestText;
    writeAiLog("AiService::sendResponseFallback using responses non-stream");
    m_client->createResponse(m_config.apiKey.trimmed(), payload);
}

void AiService::sendChatCompletionFallback()
{
    if (m_client == nullptr) {
        emit failed("AI client is not available.");
        return;
    }
    writeAiLog("AiService::sendChatCompletionFallback");

    QJsonArray messages;
    messages.append(QJsonObject{
        {"role", "system"},
        {"content", m_systemPrompt},
    });
    if (!m_inputItems.isEmpty()) {
        const QJsonObject item = m_inputItems.first().toObject();
        const QJsonArray contentArray = item.value("content").toArray();
        QString userText;
        for (const QJsonValue &contentValue : contentArray) {
            const QJsonObject contentObject = contentValue.toObject();
            if (contentObject.value("type").toString() == "input_text") {
                userText += contentObject.value("text").toString();
            }
        }
        messages.append(QJsonObject{
            {"role", "user"},
            {"content", userText},
        });
    }

    QJsonObject payload{
        {"model", m_config.model},
        {"messages", messages},
        {"stream", false},
    };

    const QString requestText =
        QString("OpenAI Request\n"
                "POST %1\n\n"
                "Payload:\n%2")
            .arg(m_config.baseUrl.toString().endsWith('/')
                     ? m_config.baseUrl.toString() + "chat/completions"
                     : m_config.baseUrl.toString() + "/chat/completions",
                 QString::fromUtf8(
                     QJsonDocument(payload).toJson(QJsonDocument::Indented)));
    if (!m_debugLog.isEmpty()) {
        m_debugLog += "\n\n";
    }
    m_debugLog += requestText;
    m_client->createChatCompletion(m_config.apiKey.trimmed(), payload);
}

void AiService::handleCompletedResponse(const QJsonObject &response)
{
    QJsonObject responseObject = unwrapResponseObject(response);
    writeAiLog(QString("AiService::handleCompletedResponse object=%1 outputSize=%2 accumulatedLen=%3")
                   .arg(responseObject.value("object").toString(),
                        QString::number(responseObject.value("output").toArray().size()),
                        QString::number(m_accumulatedResponse.size())));
    if (m_translationMode) {
        m_thinking = false;
        emit thinkingChanged(false);
        QString finalText;
        const bool isResponsesObject =
            responseObject.value("object").toString() == "response";
        const QJsonArray outputArray = responseObject.value("output").toArray();
        if (!isResponsesObject || !outputArray.isEmpty()) {
            finalText = extractResponseText(responseObject).trimmed();
        }
        if (finalText.isEmpty()) {
            finalText = m_accumulatedResponse.trimmed();
        }
        if (finalText.isEmpty() && !m_translationChatFallbackTried) {
            m_translationChatFallbackTried = true;
            m_thinking = true;
            emit thinkingChanged(true);
            sendChatCompletionFallback();
            return;
        }
        const QJsonObject object = parseTranslationObject(finalText);
        const QString description = object.value("description").toString();
        const QString inputSpec = object.value("input").toString();
        const QString outputSpec = object.value("output").toString();
        const QString hint = object.value("hint").toString();
        if (object.isEmpty()
            || (description.trimmed().isEmpty() && inputSpec.trimmed().isEmpty()
                && outputSpec.trimmed().isEmpty() && hint.trimmed().isEmpty())) {
            emit failed(QString("Translation parse failed.\n\n%1").arg(finalText));
            return;
        }
        m_translationMode = false;
        emit problemTranslationReady(description, inputSpec, outputSpec, hint);
        return;
    }

    const QJsonArray outputArray = responseObject.value("output").toArray();
    bool hasToolCall = false;
    for (const QJsonValue &itemValue : outputArray) {
        const QJsonObject itemObject = itemValue.toObject();
        const QString type = itemObject.value("type").toString();
        if (type == "function_call") {
            hasToolCall = true;
            writeAiLog(QString("AiService::handleCompletedResponse function_call name=%1")
                           .arg(itemObject.value("name").toString()));
            processToolCall(itemObject);
            break;
        }
    }

    if (!hasToolCall) {
        m_thinking = false;
        emit thinkingChanged(false);
        const QString finalText = extractResponseText(responseObject).trimmed();
        const QString streamedText = m_accumulatedResponse.trimmed();
        writeAiLog(QString("AiService::handleCompletedResponse noToolCall finalLen=%1 streamedLen=%2")
                       .arg(QString::number(finalText.size()),
                            QString::number(streamedText.size())));
        if (finalText.isEmpty() && !streamedText.isEmpty()) {
            m_translationMode = false;
            emit responseReady(streamedText);
            return;
        }
        if (finalText.isEmpty() && streamedText.isEmpty()) {
            if (!m_responseNonStreamFallbackTried) {
                m_responseNonStreamFallbackTried = true;
                m_thinking = true;
                emit thinkingChanged(true);
                writeAiLog("AiService::handleCompletedResponse empty output, fallback to non-stream responses");
                sendResponseFallback();
                return;
            }
            if (!m_chatCompletionFallbackTried) {
                m_chatCompletionFallbackTried = true;
                m_thinking = true;
                emit thinkingChanged(true);
                writeAiLog("AiService::handleCompletedResponse empty output, fallback to chat/completions");
                sendChatCompletionFallback();
                return;
            }
            emit failed("AI returned empty output.");
            return;
        }
        if (!finalText.isEmpty() && finalText != m_accumulatedResponse) {
            m_accumulatedResponse = finalText;
        }
        m_translationMode = false;
        emit responseReady(m_accumulatedResponse.isEmpty() ? finalText : m_accumulatedResponse);
        return;
    }
}

void AiService::processToolCall(const QJsonObject &callObject)
{
    const QString toolName = callObject.value("name").toString();
    const QString callId = callObject.value("call_id").toString();
    const QJsonObject args = parseArguments(callObject.value("arguments").toString());
    writeAiLog(QString("AiService::processToolCall name=%1 callId=%2 args=%3")
                   .arg(toolName,
                        callId,
                        QString::fromUtf8(QJsonDocument(args).toJson(QJsonDocument::Compact))));

    if (callId.isEmpty() || toolName.isEmpty()) {
        emit failed("Invalid tool call from model.");
        return;
    }

    if (toolName == "get_current_problem") {
        const QString value = m_toolContext.getCurrentProblem ? m_toolContext.getCurrentProblem()
                                                              : m_problemDetail;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  value,
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "get_current_language") {
        const QString value = m_toolContext.getCurrentLanguage ? m_toolContext.getCurrentLanguage()
                                                               : QString();
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  value,
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "get_current_code") {
        const QString value = m_toolContext.getCurrentCode ? m_toolContext.getCurrentCode()
                                                           : m_sourceCode;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  value,
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "get_test_input") {
        const QString value = m_toolContext.getTestInput ? m_toolContext.getTestInput()
                                                         : m_testInput;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  value,
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "get_test_output") {
        const QString value = m_toolContext.getTestOutput ? m_toolContext.getTestOutput()
                                                          : m_testOutput;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  value,
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "replace_code") {
        const QString newCode = args.value("text").toString();
        if (m_toolContext.replaceCode) {
            m_toolContext.replaceCode(newCode);
        }
        m_sourceCode = newCode;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  "Code updated.",
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "replace_input") {
        const QString newInput = args.value("text").toString();
        if (m_toolContext.replaceInput) {
            m_toolContext.replaceInput(newInput);
        }
        m_testInput = newInput;
        m_inputItems = QJsonArray{makeMessageItem(
            "user",
            buildToolResultPrompt(toolName,
                                  "Test input updated.",
                                  m_problemDetail,
                                  m_sourceCode,
                                  m_testInput,
                                  m_testOutput))};
        sendRequest();
        return;
    }
    if (toolName == "run_test") {
        QString stdinText;
        if (args.contains("stdin")) {
            stdinText = args.value("stdin").toString();
        } else if (m_toolContext.getTestInput) {
            stdinText = m_toolContext.getTestInput();
        } else {
            stdinText = m_testInput;
        }
        m_testInput = stdinText;
        m_pendingToolCallId = callId;
        m_pendingToolName = toolName;
        emit toolCallRequested(toolName, callId, args);
        if (m_toolContext.runTest) {
            m_toolContext.runTest(callId, stdinText);
        }
        return;
    }
    if (toolName == "submit_code") {
        m_pendingToolCallId = callId;
        m_pendingToolName = toolName;
        emit toolCallRequested(toolName, callId, args);
        if (m_toolContext.submitCode) {
            m_toolContext.submitCode(callId);
        } else {
            m_inputItems = QJsonArray{makeMessageItem(
                "user",
                buildToolResultPrompt(toolName,
                                      "Submission requested.",
                                      m_problemDetail,
                                      m_sourceCode,
                                      m_testInput,
                                      m_testOutput))};
            sendRequest();
        }
        return;
    }

    m_inputItems = QJsonArray{makeMessageItem(
        "user",
        buildToolResultPrompt(toolName,
                              QString("Unknown tool: %1").arg(toolName),
                              m_problemDetail,
                              m_sourceCode,
                              m_testInput,
                              m_testOutput))};
    sendRequest();
}

QString AiService::extractResponseText(const QJsonObject &response) const
{
    if (response.contains("choices")) {
        const QJsonArray choices = response.value("choices").toArray();
        if (!choices.isEmpty()) {
            const QJsonObject message =
                choices.first().toObject().value("message").toObject();
            const QString content = message.value("content").toString();
            if (!content.isEmpty()) {
                return content;
            }
        }
    }

    const QString outputText = response.value("output_text").toString();
    if (!outputText.isEmpty()) {
        return outputText;
    }

    const QJsonArray outputArray = response.value("output").toArray();
    if (response.value("object").toString() == "response" && outputArray.isEmpty()) {
        return QString();
    }
    QStringList pieces;
    for (const QJsonValue &itemValue : outputArray) {
        const QJsonObject itemObject = itemValue.toObject();
        const QJsonArray contentArray = itemObject.value("content").toArray();
        for (const QJsonValue &contentValue : contentArray) {
            const QJsonObject contentObject = contentValue.toObject();
            const QString text = contentObject.value("text").toString();
            if (!text.isEmpty()) {
                pieces << text;
            }
        }
    }
    if (!pieces.isEmpty()) {
        return pieces.join("\n\n");
    }

    return QString::fromUtf8(
        QJsonDocument(response).toJson(QJsonDocument::Indented));
}

QJsonObject AiService::parseArguments(const QString &argumentsText) const
{
    const QJsonDocument document = QJsonDocument::fromJson(argumentsText.toUtf8());
    return document.isObject() ? document.object() : QJsonObject();
}

QJsonObject AiService::parseTranslationObject(const QString &text) const
{
    const auto tryParseObject = [](const QString &candidate) {
        const QJsonDocument document =
            QJsonDocument::fromJson(candidate.trimmed().toUtf8());
        return document.isObject() ? document.object() : QJsonObject();
    };

    QJsonObject object = tryParseObject(text);
    if (!object.isEmpty()) {
        return object;
    }

    QRegularExpression fencedJson(
        QStringLiteral("```(?:json)?\\s*([\\s\\S]*?)\\s*```"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch fencedMatch = fencedJson.match(text);
    if (fencedMatch.hasMatch()) {
        object = tryParseObject(fencedMatch.captured(1));
        if (!object.isEmpty()) {
            return object;
        }
    }

    const int firstBrace = text.indexOf('{');
    const int lastBrace = text.lastIndexOf('}');
    if (firstBrace >= 0 && lastBrace > firstBrace) {
        object = tryParseObject(text.mid(firstBrace, lastBrace - firstBrace + 1));
        if (!object.isEmpty()) {
            return object;
        }
    }

    return QJsonObject();
}
