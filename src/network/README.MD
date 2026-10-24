# Network 模块

`src/network` 提供 OpenJudge 网站请求、Cookie 管理，以及向判题服务提交代码的网络接口。主要类是 `OpenJudgeClient` 和 `CookieStore`。

## 请求结果

网络操作以异步方式执行。公开请求函数返回 `void`；请求完成后通过对应的信号返回 `NetworkResult`。结果中的 `body` 是原始响应字节，不会在 network 模块中解码字符集或解析 HTML/JSON。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `ok` | `bool` | `networkError == QNetworkReply::NoError` 时为 `true`。它表示 Qt 网络层没有报告错误，不等同于业务操作成功。 |
| `statusCode` | `int` | HTTP 状态码；没有收到 HTTP 响应时通常为 `0`。应结合 `ok` 和响应内容判断结果。 |
| `networkError` | `QNetworkReply::NetworkError` | Qt 网络层错误码；无网络层错误时为 `QNetworkReply::NoError`。 |
| `errorString` | `QString` | Qt 提供的错误说明。 |
| `requestUrl` | `QUrl` | 原始请求 URL。 |
| `finalUrl` | `QUrl` | 请求完成时 reply 的 URL；发生重定向时可用于查看最终 URL。 |
| `body` | `QByteArray` | 响应正文的原始字节。 |

例如：

```cpp
connect(client, &OpenJudgeClient::problemPageFinished,
        this, [](const NetworkResult &result) {
    if (!result.ok) {
        qWarning() << result.errorString;
        return;
    }

    qDebug() << result.statusCode << result.finalUrl;
    // result.body 是原始字节；按页面实际编码解码后再交给 parser。
});
```

## `OpenJudgeClient`

### 初始化与会话

| 方法 | 作用 |
| --- | --- |
| `OpenJudgeClient(QObject *parent = nullptr)` | 创建客户端。网站基础地址默认为 `http://openjudge.cn`，判题服务默认地址为 `http://10.129.240.62:18080`。 |
| `setServerUrl(const QString &judgerBaseUrl)` | 设置判题服务基础 URL。实现会去掉首尾空白和末尾的 `/`；空字符串不会覆盖当前地址。该地址用于 `judgeSource()`，不改变 OpenJudge 网站地址。 |
| `setCookieStore(CookieStore *cookieStore)` | 将 Cookie 存储设置为网站请求使用的 cookie jar。网站请求共用 `QNetworkAccessManager`，因此可复用登录会话。 |
| `logout()` | 清空已设置的 `CookieStore`。未设置 CookieStore 时不执行操作。 |

### 登录与页面请求

这些方法使用网站请求配置（默认 15 秒超时、浏览器 User-Agent、HTML Accept 头和 Referer），并在完成时发出表中信号。登录和页面请求都通过同一个 `QNetworkAccessManager` 发出。

| 方法 | 请求 | 完成信号 |
| --- | --- | --- |
| `login(const QString &username, const QString &password)` | POST 到 `/api/auth/login/`，表单字段为 `email`、`password` 和 `redirectUrl`。 | `loginFinished(const NetworkResult &)` |
| `fetchHomePage()` | GET 网站首页。 | `homePageFinished(const NetworkResult &)` |
| `fetchUserHomePage(const QUrl &url)` | GET 指定用户主页 URL。 | `userHomePageFinished(const NetworkResult &)` |
| `fetchGroupHomePage(const QUrl &url)` | GET 指定小组主页 URL。 | `GroupHomePageFinished(const NetworkResult &)` |
| `fetchGroupPage(const QUrl &url)` | GET 指定小组页面 URL。 | `groupPageFinished(const NetworkResult &)` |
| `fetchContestPage(const QUrl &url)` | GET 指定比赛页面 URL。 | `contestPageFinished(const NetworkResult &)` |
| `fetchProblemPage(const QUrl &url)` | GET 指定题目页面 URL。 | `problemPageFinished(const NetworkResult &)` |
| `fetchResultPage(const QUrl &url)` | GET 指定提交结果页面 URL。 | `resultPageFinished(const NetworkResult &)` |
| `fetchSubmitPage(const QUrl &url)` | GET 指定提交页面 URL。 | `submitPageFinished(const NetworkResult &)` |

`GroupHomePageFinished` 的首字母大小写与其他页面信号不同，连接信号时应按头文件中的名称使用。

### 提交与判题

| 方法 | 请求与参数 | 完成信号 |
| --- | --- | --- |
| `submitSolution(const QUrl &submitActionUrl, const QByteArray &payload, const QUrl &refererUrl = QUrl())` | 对提交 action URL 发起 POST，原样发送 `payload`，并添加 `X-Requested-With: XMLHttpRequest`。未提供 Referer 时使用网站基础地址。 | `solutionSubmitted(const NetworkResult &)` |
| `judgeSource(const QString &language, const QString &fileName, const QByteArray &sourceCode, const QByteArray &stdinText, int timeLimitMs = 2000, int memoryLimitMb = 256)` | 对判题服务的 `/judge` 发起 `multipart/form-data` POST；字段包括 `language`、`stdin`、`time_limit_ms`、`memory_limit_mb` 和文件字段 `file`。 | `judgeFinished(const NetworkResult &)` |

判题请求和 OpenJudge 网站请求使用同一个 `QNetworkAccessManager`，但判题 URL 来自 `setServerUrl()`，且请求接受类型为 JSON。`judgeSource()` 仅负责传输和返回响应，不负责解释判题 JSON。

## `CookieStore`

`CookieStore` 继承自 `QNetworkCookieJar`，供 `OpenJudgeClient` 保存和复用 HTTP Cookie。它提供以下便捷方法：

| 方法 | 返回值/作用 |
| --- | --- |
| `allCookies() const` | 返回当前 jar 中的 `QList<QNetworkCookie>`。 |
| `setAllCookies(const QList<QNetworkCookie> &cookies)` | 用给定列表替换当前 Cookie 列表。 |
| `clearCookies()` | 清空 Cookie 列表。 |
| `dumpCookies() const` | 将 Cookie 的 raw form 转成 UTF-8 字符串，并以换行符连接，便于诊断输出。 |

`dumpCookies()` 可能包含会话凭据，不应将其写入公开日志或提交到版本库。

