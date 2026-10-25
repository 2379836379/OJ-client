# Login 模块

`src/login` 包含 OpenJudge 登录流程、邮箱验证码请求和本地登录信息存储。模块使用 Qt 异步网络和 SQLite；不负责登录页面的 UI 展示。

## 登录流程：`LoginService`

`LoginService` 使用 `OpenJudgeClient`。登录过程分两步：

1. `login(email, password)` 发起 OpenJudge 登录请求，并发出 `loggingChanged(true)`。
2. 登录响应为成功 JSON（`result` 字段等于 `SUCCESS`）后，再通过同一个 client 请求首页。
3. `HomepageParser::parseHomepage()` 从首页 HTML 提取个人主页 URL；成功时发出 `loginSucceeded(url)`。

| 接口/信号 | 返回值或含义 |
| --- | --- |
| `LoginService(OpenJudgeClient *client, QObject *parent = nullptr)` | 创建登录协调器。`client` 为空时，`login()` 不发起请求。 |
| `void login(const QString &email, const QString &password)` | 开始异步登录；正在登录时再次调用会忽略。 |
| `bool isLoggingIn() const` | 当前是否正在等待登录响应或登录后首页响应。 |
| `loggingChanged(bool logging)` | 登录流程开始和结束时发送状态变化。 |
| `loginSucceeded(const QUrl &personalHomeUrl)` | 登录及主页 URL 提取均成功后发送。 |
| `failed(const QString &message)` | 网络、认证响应或主页 URL 提取失败时发送错误说明。 |

登录失败信息优先取 JSON 的 `message` 字段，其次取 Qt 网络错误；两者都没有时回退为响应正文文本。

## 邮箱验证码：`EmailVerifyService`

`EmailVerifyService` 使用自身的 `QNetworkAccessManager` 直接向服务器发送 JSON POST 请求：

| 方法 | 请求 |
| --- | --- |
| `sendCode(email)` | POST 到 `{ServerUrl}/auth/send-code`，JSON 字段为 `email`。 |
| `verifyCode(email, code)` | POST 到 `{ServerUrl}/auth/login-code`，JSON 字段为 `email` 和 `code`。 |

构造时从 `AppConfig::loadServerUrl()` 读取 `network.ServerUrl`。也可通过 `setServerUrl(const QUrl &)` 设置有效的 HTTP/HTTPS URL。URL 无效或未配置时，操作会发出 `failed("Server URL is not configured.")` 并返回，不发送请求。

| 接口/信号 | 返回值或含义 |
| --- | --- |
| `bool isSending() const` | 是否正在发送验证码请求。 |
| `bool isVerifying() const` | 是否正在验证验证码请求。 |
| `sendingChanged(bool)` | 发送验证码请求开始/结束时通知状态。 |
| `verifyingChanged(bool)` | 验证码校验请求开始/结束时通知状态。 |
| `codeSent(const QString &message)` | 发送验证码请求完成且 Qt 网络层无错误时发送；优先使用 JSON `message`，否则使用默认提示。 |
| `verified(const QString &email)` | 验证码校验请求完成且 Qt 网络层无错误时发送规范化后的邮箱。 |
| `failed(const QString &message)` | 网络层错误时优先使用 JSON `error` 或 `message`，再回退网络错误文本或响应正文。 |

发送和校验共用忙碌状态，同一时间已有其中一个请求时，另一个操作会被忽略。当前校验成功条件是 Qt 网络层无错误；实现没有进一步检查响应 JSON 中的业务成功字段。

## 本地登录信息：`LoginCache`

`LoginCache` 将邮箱、密码和更新时间存入 SQLite：

- 数据库路径：`AppPaths::dataDir()/login_cache.db`。
- 表：`login_cache(email PRIMARY KEY, password, updated_at)`。
- 同一邮箱再次保存会更新密码和 `updated_at`。
- `loadLastLogin()` 按更新时间倒序读取最近保存的一条；`loadAllLogins()` 按更新时间倒序读取全部记录。

| 接口 | 返回值或行为 |
| --- | --- |
| `initialize()` | 打开数据库并创建表；成功返回 `true`，失败返回 `false`。读取方法不会自动初始化。 |
| `saveLogin(email, password)` | 保存或更新登录记录。邮箱会去首尾空白；邮箱为空时失败。 |
| `loadLastLogin(CachedLoginInfo *info)` | 找到最近登录记录时返回 `true`，并在 `info` 非空时写入 `email`、`password`、`updatedAt`。 |
| `loadLoginByEmail(email, CachedLoginInfo *info)` | 按邮箱读取记录。 |
| `loadAllLogins()` | 返回所有记录；查询失败或数据库未初始化时返回空列表。 |
| `lastError()` | 返回最近一次数据库错误或输入错误文本。 |

**安全注意：当前实现将密码以明文保存在 SQLite 数据库中。** 数据库位于用户数据目录；如需保护凭据，应改用系统凭据管理设施或加密存储，并考虑已有数据库的迁移和清理。

## 边界

- OpenJudge 账号密码登录由 `OpenJudgeClient` 处理 HTTP；`LoginService` 负责串联登录和首页解析。
- `EmailVerifyService` 使用独立的网络管理器发送验证码请求，不共享 `OpenJudgeClient` 的 Cookie 会话。
- `LoginCache` 只持久化本地输入的登录信息；它不保存或管理网站 Cookie。Cookie 会话由 `CookieStore` 和 `OpenJudgeClient` 管理。
- 登录缓存不等于页面缓存；页面缓存由 `src/cache` 负责。
