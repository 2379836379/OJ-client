#pragma once

#include "network/openjudgeclient.h"
#include "parser/submitparser.h"

#include <QObject>
#include <QUrl>

class SubmitRepository;

class SubmitService : public QObject
{
    Q_OBJECT

public:
    explicit SubmitService(SubmitRepository *repository, QObject *parent = nullptr);

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
    SubmitRepository *m_repository = nullptr;
    QUrl m_currentSubmitUrl;
    SubmitPageInfo m_currentSubmitPageInfo;
    bool m_loading = false;
    bool m_submitting = false;
};
