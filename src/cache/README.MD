# Cache 模块

`src/cache` 持久化 parser 解析后的页面数据，供上层快速读取。缓存不保存原始 HTML，也不发起网络请求、不调用 parser、不判断页面是否过期。缓存刷新时机由调用方负责。

## 存储方式

各页面缓存通过 `PageCacheStore` 使用独立的 SQLite 数据库文件。每个数据库都有一张简单的缓存表，字段为：

| 字段 | 含义 |
| --- | --- |
| `cache_key` | 主键，通常是请求页面 URL。 |
| `payload` | 缓存内容的 UTF-8 JSON 或 UTF-8 字符串，存于 SQLite BLOB。 |
| `cached_at` | 写入或覆盖缓存记录的 SQLite 时间戳。 |

保存相同 key 时使用 upsert 覆盖旧 payload 并更新时间。`cached_at` 当前只记录时间，不用于 TTL 或失效判断。缓存目录由 `AppPaths::cacheDir()` 提供；数据库文件及其 SQLite `-wal`、`-shm` 文件大小会计入缓存总量。

## 页面缓存

| 类 | 数据库文件 | Key | 缓存值 |
| --- | --- | --- | --- |
| `HomepageCache` | `homepage_cache.db` | 固定值 `homepage` | `HomepageParser::parseHomepage()` 返回的个人主页 URL 字符串。 |
| `UserHomepageCache` | `user_homepage_cache.db` | 用户主页请求 URL | `UserHomepageParser::parseUserHomepage()` 返回的 `QList<JoinedClassInfo>`，每项包含 `name` 和 `url`。 |
| `GroupHomepageCache` | `group_homepage_cache.db` | 小组主页请求 URL | `GroupHomepageParser::parseGroupHomepage()` 返回的小组 URL 字符串。 |
| `GroupPageCache` | 小组页面请求 URL | `group_page_cache.db` | `GroupPageParser::parseGroupPage()` 返回的 `GroupPageInfo::contestSets`；每项含 `url`、`title`、整数 `problemnumber` 和 `endtime`。 |
| `ContestPageCache` | `contest_page_cache.db` | 竞赛页面请求 URL | `ContestPageParser::parseContestPage()` 返回的 `ContestPageInfo::problems`；缓存保留题目列表字段，并在读取时重新计算总题数和已解决题数。 |
| `ProblemPageCache` | `problem_page_cache.db` | 题目页面请求 URL | `ProblemPageParser::parseProblemPage()` 返回的题目详情字段：`submitUrl`、时空限制、描述、starter code、输入输出说明、样例和提示。 |

### URL 与 parser 返回结构

除 `HomepageParser` 返回一个 URL 字符串外，以上 parser 返回值都不含当前被解析页面自身的 URL。因此这些缓存的 `load()`/`save()` 接口显式接收页面 URL 作为 key。不要从返回结构中读取页面 URL 来构造缓存键。

特别注意：

- `JoinedClassInfo::url` 是用户主页中某个班级/小组链接，不是用户主页自身 URL。
- `ContestSetInfo::url` 是小组页面中的竞赛或练习条目 URL，不是小组页面 URL。
- `ContestProblemInfo::problemUrl` 是竞赛中的题目 URL，不是竞赛页面 URL。
- `ProblemPageInfo::submitUrl` 是题目页解析出的提交地址，不是题目页面 URL。

## 页面缓存接口

页面缓存类不可复制。`load(...)` 在查到记录且 payload 格式有效时返回 `true`，并将结果写入非空输出指针；未命中、数据库错误或 payload 不合法时返回 `false`。空列表也可以是有效缓存，因此是否命中由 key 记录和 payload 解析决定，而不是列表是否为空。

| 类 | 读取 | 写入 |
| --- | --- | --- |
| `HomepageCache` | `bool load(QString *personalHomepageUrl)` | `bool save(const QString &personalHomepageUrl)` |
| `UserHomepageCache` | `bool load(const QString &userHomepageUrl, QList<JoinedClassInfo> *classes)` | `bool save(const QString &userHomepageUrl, const QList<JoinedClassInfo> &classes)` |
| `GroupHomepageCache` | `bool load(const QString &homepageUrl, QString *groupUrl)` | `bool save(const QString &homepageUrl, const QString &groupUrl)` |
| `GroupPageCache` | `bool load(const QString &groupPageUrl, GroupPageInfo *pageInfo)` | `bool save(const QString &groupPageUrl, const GroupPageInfo &pageInfo)` |
| `ContestPageCache` | `bool load(const QString &contestPageUrl, ContestPageInfo *pageInfo)` | `bool save(const QString &contestPageUrl, const ContestPageInfo &pageInfo)` |
| `ProblemPageCache` | `bool load(const QString &problemPageUrl, ProblemPageInfo *pageInfo)` | `bool save(const QString &problemPageUrl, const ProblemPageInfo &pageInfo)` |

每个页面缓存还提供：

- `qint64 cacheSizeBytes() const`：返回对应数据库及 `-wal`、`-shm` 文件的字节数。
- `bool clearCache()`：关闭该实例的数据库连接并删除对应数据库及 SQLite 辅助文件。成功返回 `true`；删除失败返回 `false`。
- `QString lastError() const`：返回最近一次数据库或清理错误。缓存未命中和 payload 格式错误不是数据库错误，不一定会更新该字段。

## `CacheService`

`CacheService` 聚合各页面缓存的管理操作，不负责页面数据的读写流程：

| 接口 | 返回/行为 |
| --- | --- |
| `totalCacheSizeBytes() const` | 汇总传入各缓存对象的 `cacheSizeBytes()`。 |
| `formattedTotalCacheSize() const` | 将总字节数格式化为 B、KB、MB、GB 或 TB。 |
| `clearAllCaches()` | 依次清理传入的缓存。全部成功时发出 `cacheCleared()` 并返回 `true`；有失败时汇总错误、发出 `failed(message)` 并返回 `false`。 |
| `lastError() const` | 返回上次整体清理失败的错误信息。 |

构造时传入空指针的缓存会在统计和清理时跳过。清理缓存不会清除登录凭据、收藏、题目笔记或 AI 配置；它们不属于 `src/cache` 中的页面缓存数据库。

## 缓存流程约定

典型调用顺序由上层页面/功能协调者实现：先用请求 URL 调用缓存 `load()`，命中后可先展示；然后再请求网络并解析；网络结果有效时用相同请求 URL 作为 key 调用 `save()`。缓存类本身不执行这套流程，也不会因为请求失败自动删除旧数据。
