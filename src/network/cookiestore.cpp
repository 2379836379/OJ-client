#include "cookiestore.h"

#include <QStringList>

CookieStore::CookieStore(QObject *parent)
    : QNetworkCookieJar(parent)
{
}

QList<QNetworkCookie> CookieStore::allCookies() const
{
    return QNetworkCookieJar::allCookies();
}

void CookieStore::setAllCookies(const QList<QNetworkCookie> &cookies)
{
    QNetworkCookieJar::setAllCookies(cookies);
}

void CookieStore::clearCookies()
{
    QNetworkCookieJar::setAllCookies({});
}

QString CookieStore::dumpCookies() const
{
    QStringList lines;
    const QList<QNetworkCookie> cookies = QNetworkCookieJar::allCookies();
    for (const QNetworkCookie &cookie : cookies) {
        lines << QString::fromUtf8(cookie.toRawForm());
    }
    return lines.join('\n');
}
