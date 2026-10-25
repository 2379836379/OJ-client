#pragma once

#include "network/openjudgeclient.h"
#include "parser/SubmitPage.h"

#include <QObject>
#include <QUrl>

class SubmitService : public QObject
{
    Q_OBJECT

public:
    explicit SubmitService(OpenJudgeClient *client, QObject *parent = nullptr);

    void openSubmit(const QUrl &submitPageUrl);
    void submitSolution(const QString &language, const QString &sourceText);

    QUrl currentSubmitUrl() const;
    SubmitPageInfo currentSubmitPageInfo() const;
    QString defaultLanguage() const;

signals:
    void loadingChanged(bool loading);
    void submittingChanged(bool submitting);
    void submitPageLoaded(const SubmitPageInfo &submitPageInfo);
    void submitPayloadBuilt(const QString &languageValue,
                            const QString &sourceText,
                            const QByteArray &payload);
    void solutionSubmitted(const NetworkResult &result);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_currentSubmitUrl;
    SubmitPageInfo m_currentSubmitPageInfo;
    QUrl m_expectedSubmitUrl;
    QUrl m_expectedActionUrl;
};
