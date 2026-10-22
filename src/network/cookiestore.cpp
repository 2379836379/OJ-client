#include "cookiestore.h"

#include <QSet>
#include <QStringList>

CookieStore::CookieStore(QObject *parent)
    : QNetworkCookieJar(parent)
{
}

QList<QNetworkCookie> CookieStore::allCookies() const
{
    return cookiesForUrl(QUrl("http://openjudge.cn"));
}

void CookieStore::setAllCookies(const QList<QNetworkCookie> &cookies)
{
    setCookiesFromUrl(cookies, QUrl("http://openjudge.cn"));
}

void CookieStore::clearCookies()
{
    setAllCookies({});
}

QString CookieStore::dumpCookies() const
{
    QStringList lines;
    const QList<QNetworkCookie> cookies = allCookies();
    for (const QNetworkCookie &cookie : cookies) {
        lines << QString::fromUtf8(cookie.toRawForm());
    }
    return lines.join('\n');
}

QList<QNetworkCookie> CookieStore::cookiesForUrl(const QUrl &url) const
{
    QList<QNetworkCookie> cookies = QNetworkCookieJar::cookiesForUrl(url);
    const QString host = url.host().toLower();
    if (host == "openjudge.cn" || !host.endsWith(".openjudge.cn")) {
        return cookies;
    }

    const QList<QNetworkCookie> rootCookies =
        QNetworkCookieJar::cookiesForUrl(QUrl("http://openjudge.cn"));
    QSet<QByteArray> seen;
    for (const QNetworkCookie &cookie : cookies) {
        seen.insert(cookie.name() + '\n' + cookie.path().toUtf8());
    }

    for (const QNetworkCookie &cookie : rootCookies) {
        const QByteArray key = cookie.name() + '\n' + cookie.path().toUtf8();
        if (seen.contains(key)) {
            continue;
        }
        cookies.append(cookie);
        seen.insert(key);
    }

    return cookies;
}
