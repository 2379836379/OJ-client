#include "submitservice.h"

SubmitService::SubmitService(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::submitPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedSubmitUrl.isEmpty()
                && result.requestUrl != m_expectedSubmitUrl) {
                return;
            }
            m_expectedSubmitUrl = {};
            if (!result.ok) {
                emit loadingChanged(false);
                emit failed(result.errorString);
                return;
            }

            m_currentSubmitPageInfo =
                SubmitPageParser::parseSubmitPage(result.body, result.finalUrl);
            emit loadingChanged(false);
            emit submitPageLoaded(m_currentSubmitPageInfo);
        });

    connect(
        m_client,
        &OpenJudgeClient::solutionSubmitted,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedActionUrl.isEmpty()
                && result.requestUrl != m_expectedActionUrl) {
                return;
            }
            m_expectedActionUrl = {};
            emit submittingChanged(false);
            if (!result.ok) {
                emit failed(result.errorString);
            } else {
                emit solutionSubmitted(result);
            }
        });
}

void SubmitService::openSubmit(const QUrl &submitPageUrl)
{
    m_currentSubmitUrl = submitPageUrl;
    m_currentSubmitPageInfo = {};
    m_expectedSubmitUrl = submitPageUrl;
    emit loadingChanged(true);
    m_client->fetchSubmitPage(submitPageUrl);
}

void SubmitService::submitSolution(const QString &language,
                                   const QString &sourceText)
{
    if (m_currentSubmitPageInfo.submitActionUrl.isEmpty()) {
        emit failed("submit action url not found");
        return;
    }

    const QByteArray payload = SubmitPageParser::buildSubmitPayload(
        m_currentSubmitPageInfo, language, sourceText);
    emit submitPayloadBuilt(language, sourceText, payload);
    emit submittingChanged(true);
    const QUrl actionUrl(m_currentSubmitPageInfo.submitActionUrl);
    m_expectedActionUrl = actionUrl;
    m_client->submitSolution(actionUrl, payload, m_currentSubmitUrl);
}

QUrl SubmitService::currentSubmitUrl() const
{
    return m_currentSubmitUrl;
}

SubmitPageInfo SubmitService::currentSubmitPageInfo() const
{
    return m_currentSubmitPageInfo;
}

QString SubmitService::defaultLanguage() const
{
    return SubmitPageParser::defaultLanguage(m_currentSubmitPageInfo);
}
