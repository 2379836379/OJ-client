#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct SubmitLanguageOption
{
    QString value;
    QString label;
    bool checked = false;
};

struct SubmitPageInfo
{
    QString pageUrl;
    QString submitActionUrl;
    QString contestId;
    QString problemNumber;
    QString sourceEncode;
    QList<SubmitLanguageOption> languages;
};

namespace SubmitParser
{
SubmitPageInfo parseSubmitPage(const QByteArray &html, const QUrl &baseUrl);
QString defaultLanguage(const SubmitPageInfo &pageInfo);
bool hasLanguage(const SubmitPageInfo &pageInfo, const QString &language);
QByteArray buildSubmitPayload(const SubmitPageInfo &pageInfo,
                              const QString &language,
                              const QString &sourceText = "test");
}
