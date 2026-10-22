#pragma once

#include <QList>
#include <QNetworkCookie>
#include <QNetworkCookieJar>

class CookieStore : public QNetworkCookieJar
{
    Q_OBJECT

public:
    explicit CookieStore(QObject *parent = nullptr);

    QList<QNetworkCookie> allCookies() const;
    void setAllCookies(const QList<QNetworkCookie> &cookies);
    void clearCookies();
    QString dumpCookies() const;

protected:
    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const override;
};
