# Parser 接口与返回值

本目录负责将 OpenJudge 页面 HTML 解析为 Qt 字符串、列表和信息结构。HTML 参数使用 `QByteArray`，文档由 Lexbor 解析。解析不到的字符串通常为空；列表为空；计数和布尔字段保持结构体中的默认值。

## Homepage

```cpp
QString HomepageParser::parseHomepage(const QByteArray &html);
```

从首页查找文字为“个人首页”的链接，并返回其 URL。仅接受 `openjudge.cn/user/<数字>/` 的 HTTP/HTTPS URL。未找到符合条件的链接时返回空字符串。该接口不接收 base URL，因此链接应为绝对 URL。

## GroupHomepage

```cpp
QString GroupHomepageParser::parseGroupHomepage(const QByteArray &html);
```

查找文字为“前往小组”的 `<a>`，返回其 `href` 属性原值；未找到时返回空字符串。

## UserHomepage

```cpp
QList<JoinedClassInfo>
UserHomepageParser::parseUserHomepage(const QByteArray &html);
```

返回主页中带有 `group-name` class 的链接列表：

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `JoinedClassInfo::name` | `QString` | 链接内的可见文本，空白已折叠 |
| `JoinedClassInfo::url` | `QString` | 链接 `href` 原值 |

没有名称或 URL 的链接会被忽略；没有匹配项时返回空列表。

## GroupPage

```cpp
GroupPageInfo GroupPageParser::parseGroupPage(
    const QByteArray &html, const QUrl &baseUrl);
```

读取 `current-contest` 列表中的 `contest-info` 和 `practice-info` 项。`baseUrl` 用于解析条目的相对 `href`。`GroupPageInfo` 包含 `QList<ContestSetInfo> contestSets`。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `ContestSetInfo::url` | `QString` | 相对于 `baseUrl` 解析后的条目 URL |
| `ContestSetInfo::title` | `QString` | 标题链接文本 |
| `ContestSetInfo::problemnumber` | `int` | 标题链接后的题数文本中的数字，例如 `(5题)` 返回 `5`；没有数字时为 `0` |
| `ContestSetInfo::endtime` | `QString` | `over-time` 文本中“结束时间：”之后的内容；没有该格式时为空。练习项的“最新题目”不作为结束时间返回 |

## ContestPage

```cpp
ContestPageInfo ContestPageParser::parseContestPage(
    const QByteArray &html, const QUrl &baseUrl);
```

读取竞赛表格中同时含题号和标题链接的行。`baseUrl` 用于解析题目链接。

| `ContestPageInfo` 字段 | 类型 | 含义 |
| --- | --- | --- |
| `problems` | `QList<ContestProblemInfo>` | 题目列表 |
| `totalProblems` | `int` | 成功解析的题目数 |
| `solvedProblems` | `int` | 列表中标记为已解决的题目数 |

每个 `ContestProblemInfo` 的字段：

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `problemId` | `QString` | 题号链接文本 |
| `title` | `QString` | 标题链接文本 |
| `problemUrl` | `QString` | 标题链接相对于 `baseUrl` 解析后的 URL |
| `accept_prople` | `int` | `accepted` 单元格中的通过人数；无法转换为整数时为 `0` |
| `submission_people` | `int` | `submissions` 单元格中的尝试人数；无法转换为整数时为 `0` |
| `solved` | `bool` | `solved` 单元格图片的 `src` 包含 `accepted.gif` 时为 `true` |

## ProblemPage

```cpp
ProblemPageInfo ProblemPageParser::parseProblemPage(
    const QByteArray &html, const QUrl &baseUrl);
```

从题目详情页读取题目说明、限制、输入输出、样例和提示。`baseUrl` 用于解析固定相对路径 `submit/`，作为提交页 URL 的基准。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `submitUrl` | `QString` | 相对于 `baseUrl` 解析出的 `submit/` URL |
| `timeLimit` | `QString` | “总时间限制”对应的 `<dd>` 文本 |
| `memoryLimit` | `QString` | “内存限制”对应的 `<dd>` 文本 |
| `description` | `QString` | “描述”对应 `<dd>` 的纯文本 |
| `starterCode` | `QString` | 描述区中所有 `<pre>` 文本，多个块以空行分隔 |
| `inputSpec` | `QString` | “输入”对应的 `<dd>` 文本 |
| `outputSpec` | `QString` | “输出”对应的 `<dd>` 文本 |
| `sampleInput` | `QString` | “样例输入”对应的 `<dd>` 文本 |
| `sampleOutput` | `QString` | “样例输出”对应的 `<dd>` 文本 |
| `hint` | `QString` | “提示”对应的 `<dd>` 文本 |

普通文本取值会折叠连续空白；`starterCode` 保留 `<pre>` 内部换行并去除首尾空白。当前结构没有题目标题、尝试人数或通过人数返回字段。

## ResultPage

```cpp
ResultPageInfo ResultPageParser::parseResultPage(const QByteArray &html);
```

读取结果页中的编译/运行状态和 `h3-compile-status` 之后的首个 `<pre>` 内容。该接口不需要 base URL，也不返回提交 ID 或结果链接。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `statusText` | `QString` | `compile-status` 段落内链接的文本 |
| `detailTitle` | `QString` | `h3-compile-status` 标题文本 |
| `detailText` | `QString` | 该标题后首个 `<pre>` 的文本，去除首尾空白 |
| `hasResult` | `bool` | 状态文本非空时为 `true` |

## SubmitPage

```cpp
SubmitPageInfo SubmitPageParser::parseSubmitPage(
    const QByteArray &html, const QUrl &baseUrl);
QString SubmitPageParser::defaultLanguage(const SubmitPageInfo &pageInfo);
bool SubmitPageParser::hasLanguage(
    const SubmitPageInfo &pageInfo, const QString &language);
QByteArray SubmitPageParser::buildSubmitPayload(
    const SubmitPageInfo &pageInfo,
    const QString &language,
    const QString &sourceText = "test");
```

`parseSubmitPage` 从 `solution_submit` 表单读取提交动作、隐藏字段和语言单选项。`baseUrl` 用于解析表单的相对 `action`。

| `SubmitPageInfo` 字段 | 类型 | 含义 |
| --- | --- | --- |
| `submitActionUrl` | `QString` | 表单 `action` 相对于 `baseUrl` 解析后的 URL |
| `contestId` | `QString` | 名为 `contestId` 的隐藏字段值 |
| `problemNumber` | `QString` | 名为 `problemNumber` 的隐藏字段值 |
| `sourceEncode` | `QString` | 名为 `sourceEncode` 的隐藏字段值 |
| `languages` | `QList<SubmitLanguageOption>` | 语言单选项列表 |

每个 `SubmitLanguageOption` 的字段：`value` 是表单提交值，`label` 是页面显示文本，`checked` 表示 HTML 是否标记该选项为选中。

- `defaultLanguage` 返回第一个选中的语言；没有选中项时返回第一项；列表为空时返回空字符串。
- `hasLanguage` 检查语言 `value` 是否存在于选项列表中。
- `buildSubmitPayload` 生成 URL 编码的表单数据：`contestId`、`problemNumber`、`sourceEncode`、`language`、`source`。传入语言不存在时回退到默认语言；源代码按 UTF-8 编码后进行 Base64，再进行表单百分号编码。

提交请求的 Cookie、Referer 和 `X-Requested-With` 请求头由网络层负责，不属于 parser 返回值或 payload 字段。
