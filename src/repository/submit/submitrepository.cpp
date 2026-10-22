#include "submitrepository.h"

#include "network/openjudgeclient.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

namespace
{
void writeStartupLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | " << message << '\n';
}
}

SubmitRepository::SubmitRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::submitPageFinished,
        this,
        [this](const NetworkResult &result) {
            writeStartupLog("SubmitRepository: submitPageFinished received");
            if (!m_expectedSubmitPageUrl.isEmpty()
                && result.finalUrl != m_expectedSubmitPageUrl) {
                writeStartupLog("SubmitRepository: submitPageFinished ignored due to finalUrl mismatch");
                return;
            }

            m_expectedSubmitPageUrl = QUrl();
            if (!result.ok) {
                writeStartupLog("SubmitRepository: submitPageFinished network failure");
                emit requestFailed("submitPage", result.errorString);
                return;
            }

            writeStartupLog("SubmitRepository: parsing submit page begin");
            emit submitPageFetched(
                SubmitParser::parseSubmitPage(result.body, result.finalUrl));
            writeStartupLog("SubmitRepository: parsing submit page end");
        });

    connect(
        m_client,
        &OpenJudgeClient::solutionSubmitted,
        this,
        [this](const NetworkResult &result) {
            m_expectedSubmitActionUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("submitSolution", result.errorString);
                return;
            }

            emit solutionSubmitted(result);
        });
}

void SubmitRepository::fetchSubmitPage(const QUrl &submitPageUrl)
{
    writeStartupLog("SubmitRepository::fetchSubmitPage begin");
    m_expectedSubmitPageUrl = submitPageUrl;
    m_client->fetchSubmitPage(submitPageUrl);
    writeStartupLog("SubmitRepository::fetchSubmitPage client fetch called");
}

void SubmitRepository::submitSolution(const QUrl &submitActionUrl,
                                      const QByteArray &payload,
                                      const QUrl &refererUrl)
{
    m_expectedSubmitActionUrl = submitActionUrl;
    m_client->submitSolution(submitActionUrl, payload, refererUrl);
}
