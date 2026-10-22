#include "submitservice.h"

#include "repository/submit/submitrepository.h"

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

SubmitService::SubmitService(SubmitRepository *repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    connect(
        m_repository,
        &SubmitRepository::submitPageFetched,
        this,
        [this](const SubmitPageInfo &submitPageInfo) {
            writeStartupLog("SubmitService: submitPageFetched received");
            m_currentSubmitPageInfo = submitPageInfo;
            m_loading = false;
            emit loadingChanged(false);
            writeStartupLog("SubmitService: loadingChanged(false) emitted");
            emit submitPageLoaded(submitPageInfo);
            writeStartupLog("SubmitService: submitPageLoaded emitted");
        });

    connect(
        m_repository,
        &SubmitRepository::solutionSubmitted,
        this,
        [this](const NetworkResult &result) {
            m_submitting = false;
            emit submittingChanged(false);
            emit solutionSubmitted(result);
        });

    connect(
        m_repository,
        &SubmitRepository::requestFailed,
        this,
        [this](const QString &stage, const QString &message) {
            if (stage == "submitPage") {
                m_loading = false;
                emit loadingChanged(false);
            } else if (stage == "submitSolution") {
                m_submitting = false;
                emit submittingChanged(false);
            }
            emit failed(message);
        });
}

void SubmitService::openSubmit(const QUrl &submitPageUrl)
{
    writeStartupLog("SubmitService::openSubmit begin");
    m_currentSubmitUrl = submitPageUrl;
    m_loading = true;
    emit loadingChanged(true);
    writeStartupLog("SubmitService::openSubmit loading emitted");
    m_repository->fetchSubmitPage(submitPageUrl);
    writeStartupLog("SubmitService::openSubmit repository fetch called");
}

void SubmitService::submitSolution(const QString &language,
                                   const QString &sourceText)
{
    if (m_currentSubmitPageInfo.submitActionUrl.isEmpty()) {
        emit failed("submit action url not found");
        return;
    }

    const QByteArray payload = SubmitParser::buildSubmitPayload(
        m_currentSubmitPageInfo, language, sourceText);
    emit submitPayloadBuilt(language, sourceText, payload);
    m_submitting = true;
    emit submittingChanged(true);
    m_repository->submitSolution(
        QUrl(m_currentSubmitPageInfo.submitActionUrl),
        payload,
        QUrl(m_currentSubmitPageInfo.pageUrl));
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
    return SubmitParser::defaultLanguage(m_currentSubmitPageInfo);
}
