# Oj-client：面向OpenJudge的桌面刷题管理器

## 摘要

oj-client 是一个基于 Qt 6 Widgets 的桌面应用，目标是把 OpenJudge 上分散在网页中的做题流程集中到一个本地客户端里完成。它覆盖网址基础功能，外加本地判题、AI 辅助，并在此基础上补齐了网页端缺失的一环——对个人刷题过程的长期管理，包括标签、笔记、任务看板、统计、错题本和基于间隔重复（SRS）的复习排程。  
项目采用「ui—业务—数据—网络/解析」四层结构，本地状态统一存放在 SQLite 数据库中，网络访问全部异步执行。本报告说明项目的背景与定位、需求、总体设计、关键模块实现，并讨论现有局限与后续方向。  

## 一、引言

OpenJudge 是一个以网页为主要交互形式的在线评测平台。日常使用中，做题相关的信息分散在不同页面：题目列表、题面、提交记录各在一处，平台本身不提供「哪些题值得重刷」这类个人维度的记录。使用者往往要靠浏览器书签、本地文件夹或额外的笔记工具自行维护。

本项目的出发点不是再造一个代码编辑器，而是把重心放在**刷题过程的管理**上：在保证基本功能的基础上，为每道题补充可长期积累的本地信息，并提供看板、统计、错题本、复习等视图，帮助使用者安排和复盘自己的练习。

项目想法来源于开源项目 Never-Judge。与其偏重「写代码并提交」的定位不同，本项目主动收窄了编辑器方向的投入，把资源投向任务管理与本地数据组织。主要面向需要长期在 OpenJudge 上练习的学生与算法练习者，以及希望对题目做本地归档、整理标签与复盘笔记的使用者。

## 二、功能汇总

| 分类 | 主要功能 |
| --- | --- |
| 账户与登录 | 账号密码登录、会话 Cookie 维持、记住最近账号、启动时恢复登录、邮箱验证码流程 |
| 信息浏览 | 课程列表、比赛列表、题目列表、做题状态标记 |
| 题目页 | 题面展示、提交区、本地测试区、AI 区、收藏入口、Notes 侧栏 |
| 代码与提交 | 代码编辑、语法高亮、草稿缓存、语言选择、提交与结果查看 |
| 本地测试 | 调用独立 judge 服务运行当前代码，按语言映射文件名 |
| AI 辅助 | 读取题面/代码/测试数据、替换代码、触发测试与提交、题面翻译、预设提示 |
| 收藏 | 收藏夹管理、离线保存题面、JSON 导出/导入 |
| 刷题管理 | 标签、笔记、任务状态、难度、错题本、任务看板、统计、SRS 复习 |
| 提醒 | 解析截止时间、Due Soon 展示、定时闹钟与本地响铃、系统托盘通知 |


## 三、项目结构

### 3.1 目录结构

```text
oj-client/
├─ src/
│  ├─ config/        配置读写与路径工具
│  ├─ network/       OpenJudge / OpenAI / judge / 邮箱验证访问
│  ├─ parser/        HTML 与结果解析
│  ├─ repository/    本地数据库与远端仓库封装
│  ├─ service/       业务逻辑层
│  └─ ui/            主窗口与各页面
├─ images/           UI 图标资源
├─ vedio/            铃声音频资源
├─ third_party/      tree-sitter 及语法库
├─ tests/            解析器单元测试
├─ installer.iss     Inno Setup 安装脚本
├─ CMakeLists.txt    CMake 构建文件
├─ config.toml       AI 配置
└─ appstate.toml     运行时状态配置
```

### 3.2 架构分层

系统自上而下分为 UI、业务、数据、网络/解析 共四层，分层清晰，职责单一，便于增加新功能和维护。

#### 3.2.1 UI 层

UI 层位于 `src/ui`，负责所有页面和主窗口展示。当前主要页面以下页面：

- `LoginPage`：登录页。
- `HomePage`：首页，展示课程与 due soon 比赛。
- `ClassPage`：课程页。
- `ContestPage`：比赛页。
- `ProblemPage`：题目页，是最核心的工作区。  

此外还有诸多页面，每个页面中的功能都在对应页面里实现，UI 层只负责显示、交互和信号转发，与SQL存储逻辑和网络请求无关。

#### 3.2.2 Service 层

Service 层位于 `src/service`，负责业务处理，这一层把 UI 与底层数据访问分开，避免页面类中混入大量网络和数据库代码,提供给前端可以直接调用的服务接口。

- `LoginService`：登录与登录缓存逻辑。
- `HomeService` / `ClassService` / `ContestService` / `ProblemService`：浏览类业务。
- `SubmitService` / `ResultService`：提交与评测结果获取。
- `AiService`：AI 请求、工具调用、翻译与降级逻辑。
- `CacheService`：缓存清理。
- `ApplicationSizeService`：应用与缓存大小统计。
- `ReminderService` / `DeadlineAlarmService`：截止提醒与闹钟触发。
- `FavoriteProblemService`：收藏相关业务。

#### 3.2.3 Repository 层

Repository 层位于 `src/repository`，负责组织本地数据与远程抓取结果，直接操作数据库，把数据读写细节屏蔽起来，对service层提供稳定接口，典型模块有：

- `favorite`：收藏夹 SQLite 仓库。
- `cache`：课程、比赛、题目等缓存仓库。
- `submit`：与远程题目、提交、结果页对应的数据封装。
- `login`：登录缓存仓库。
- `meta`：题目元数据仓库。

#### 3.2.4 Network 与 Parser 层

- Network层位于`src/network`：负责所有 HTTP 请求，包括 OpenJudge、OpenAI 兼容接口、本地 judge 服务等。  
- Parser层位于`src/parser`：负责把 HTML 页面解析成结构化数据，由于openjudge网站的内容和结构相对稳定，故此处使用正则表达式即可完成解析。


## 四、制作的额外配套工具

### 4.1 [Oj-judger](https://github.com/2379836379/OJ-judger)

    详情可见：https://github.com/2379836379/OJ-judger

oj-judger 是一个沙箱判题系统，构建于开源项目 criyle/go-judge 之上。  

go-judge 是一个使用 Go 开发的高性能、安全代码执行引擎，提供 HTTP、gRPC 和 WebSocket 等接口，在基于 Linux Namespace、cgroup、seccomp 等机制构建的沙箱环境中完成程序的编译与运行，并对 CPU 时间、内存、进程数、输出大小等资源进行限制。其基础镜像不包含对特定语言的支持，故在项目的基础镜像的基础上附加语言支持即可作为测试镜像。

oj-judger包含各语言的判题镜像和一个调度器，其接收源代码提交，选择合适的镜像在 go-judge 中运行提交，并将结果以 JSON 格式返回，目前支持的语言包括C++、Python、Rust。调度器负责流量控制机制，将请求在执行前放入队列排队，对全局并发判题任务数进行限制，若过载请求返回 503 Service Unavailable。  

当前接口包括：  
```bash
GET /healthz  
# 返回 scheduler 健康状态和队列状态。  
POST /judge   
# 开始测试，需要填写的字段：编程语言名称; 源代码文件; 标准输入内容; 
# 时间限制（ms），默认 2000; 内存限制（MB），默认 256
```

启动服务需要先构建语言镜像，依次进入各语言目录，执行 Docker 构建：
```bash
cd ./language/c++		
sudo docker build -t oj-judger:cpp 
cd ./language/python	
sudo docker build -t oj-judger:python 
cd ./language/rust		
sudo docker build -t oj-judger:rust
``` 
之后即可启动,默认监听端口:18080
```bash
sudo go run ./cmd/scheduler
```

### 4.2 [Oj-server](https://github.com/2379836379/OJ-server)

    详情可见：https://github.com/2379836379/OJ-server

oj-server是一个用于在登录时邮箱验证码认证的程序，通过 SMTP协议发送邮件。提交验证请求之后程序生成六位验证码，过期时间五分钟，首次登录时进行邮箱验证，通过后账号信息进入本地缓存，再次登录无需验证。  
接口：  
```bash
GET /healthz 		        # 健康检查    
POST /auth/send-code 		# 发送验证码到指定邮箱     
POST /auth/login-code 	    # 使用验证码登录  
```
配置项：  
```bash
export OJ_SERVER_HTTP_ADDR=:9000                #自定义服务端口,默认8080
export OJ_SERVER_SQLITE_PATH=/tmp/oj-server.db  #默认 SQLite 数据库路径
export OJ_SMTP_HOST=smtp.example.com            #SMTP 服务器地址
export OJ_SMTP_PORT=587                         #SMTP 端口
export OJ_SMTP_USER=mailer@example.com          #SMTP 用户名
export OJ_SMTP_PASS=your-password               #SMTP 密码
export OJ_SMTP_FROM=mailer@example.com          #发件人邮箱
export OJ_SMTP_TLS_MODE=starttls                
#传输层安全协议，支持starttls，tls或none
```
启动  
```
go run ./cmd/server
```

## 五、关键模块实现

### 5.1 登录、邮件验证和cookie

输入邮箱和密码进行登录，首次登录需要进行邮件验证，调用oj-server完成。
验证之后，OpenJudgeClient::login() 会向：openjudge的登录接口`/api/auth/login/`发送表单 POST，请求体里包含：
  - `email`
  - `password`
  - `redirectUrl`  

请求通过 `QNetworkAccessManager` 发出，cookie 也会进入 `CookieStore`，所以后续页面请求能继承会话。

### 5.2 获取题目信息
#### 5.2.1 Problempage
从主界面有多种方式到达题目页
- 从HomePage开始逐级访问，HomePage -> ClassPage -> ContestPage -> ProblemPage 显示题面。
- 从due soon列表进入contest ，再进入problempage
- 从收藏/错题本/笔记进入，此时本地收藏里已经有完整题面，会优先直接显示缓存的题面，不走网络。

#### 5.2.2 Service过程

访问题目的请求到达service，先检查`ProblemCacheRepository`，无论有没有缓存，都会继续发起网络请求并启动Repository过程。采用的是“缓存先显示，网络再刷新”的策略：
- 有缓存：用户立刻看到旧数据，界面不空白。
- 网络返回后：如果内容有变化，再覆盖成最新题面，并更新缓存。

#### 5.2.3 Network过程

调用`m_manager.get(createRequest(url))`发出请求, 收集reply到`connectReply()`，最后发出 `problemPageFinished(result)`信号。  
`createRequest()` 会统一设置请求头:  
```
- User-Agent
- Content-Type: application/x-www-form-urlencoded
- Accept: text/html...
- Referer
- Connection: keep-alive
```

程序还附加 `CookieStore` 到 `QNetworkAccessManager` 上，登录后的会话 cookie 会自动附加到题目请求。避免在丢失登录状态下，openjudge强制跳转到登录界面导致解析失败。  
`connectReply()`当 reply finished 时，它会统一提取下列信息，最后把这些装进 `NetworkResult`，再交给上层：
```
- statusCode
- networkError
- errorString
- requestUrl
- finalUrl
- body
- ok
```

#### 5.2.4 Repository过程

记录题目 URL，调用 `OpenJudgeClient::fetchProblemPage`，监听`OpenJudgeClient::problemPageFinished` 信号,收到访问完成的响应后会执行：
1. 检查 `result.finalUrl` 确认是否还是自己请求的那道题。这里的 finalUrl 检查很重要。因为 Qt 网络请求可能发生跳转，如果连续点两道题，旧请求晚回来就可能污染当前页面。这个判断是防串页保护。
2. 如果网络失败，发出信号 `requestFailed("problemPage", errorString)`。
3. 如果成功，调用 `ProblemParser::parseProblemPage(result.body, result.finalUrl)`，把 HTML 解析成结构化的 `ProblemPageInfo`，再发出信号 `problemFetched(problemPageInfo)` 表示解析完成。

#### 5.2.5 Parser过程
`ProblemParser::parseProblemPage()`负责解析题目页面 HTML。
1. `ParserCommon::toUtf8String(html)` 先把网页字节流转成字符串。
2. 提取标题和提交地址：  
3. 从 div id="pageTitle" 里的 h2 提取 title；从页面里的 /submit/ 链接，解析成绝对 submitUrl，用 `<dt>...</dt><dd>...</dd>` 结构提取：
```
    - timeLimit
    - memoryLimit
    - description
    - inputSpec
    - outputSpec
    - sampleInput
    - sampleOutput
    - hint
```
4. 对于有初始代码的情况，`extractPreBlocksText()` 会：把所有 `<pre>...</pre>` 内容抓出来做 HTML 实体还原,之后去掉空块，按空行拼接。所以程序填空题里的前置代码、后置代码，都能被收集成 starterCode，打开提交页时代码框能自动带模板。


### 5.3 测试
为了避免本地测试时用户需要安装对于语言的环境，故使用oj-judger实现。点击input时，向本地 judge 服务地址发送 multipart/form-data 请求。

judge 返回结果后，OpenJudgeClient 用 `connectReply()` 统一把返回封成 `NetworkResult`，再发出：`judgeFinished(result)`信号，MainWindow 监听这个信号来做结果展示。

收到 judgeFinished 后，主窗口会：先结束测试状态：再判断结果是否成功，如果失败，调用 `showTestFailed(...)`显示测试失败的原因（不是程序错误信息），如果成功，调用 `showTestResult(...)`，显示程序的输出或程序错误信息，如编译错误信息。


### 5.4 代码编辑与语法高亮

提交区代码框做成轻量 IDE 风格，功能包括：独立行号列、成对括号自动补全、回车继承缩进、`{` 后回车生成块级缩进。语法高亮基于 Tree-sitter，接入 `tree-sitter-cpp` 与 `tree-sitter-python`，随提交语言自动切换。

### 5.5 AI 助手

`config.toml`内可配置 OpenAI 兼容接口。AI 能读取当前题面、语言、代码、测试输入输出，并可替换代码、替换测试输入、触发本地测试与提交。题面翻译覆盖 Description / Input / Output / Hint，结果缓存后再次点击优先显示缓存，不重复请求。题目页 AI 面板提供三个预设提示按钮（解释题意、写暴力解、优化复杂度），点击后把对应提示填入输入框并聚焦，减少重复输入。

### 5.6 收藏系统与 JSON 导入导出

收藏支持创建收藏夹、保存当前题、数据本地化。收藏页支持 Export / Import：导出把全部收藏夹及题目序列化为带版本号的 JSON 文件；导入按同名文件夹合并、按题目网址 `problemUrl` 去重写入，避免重复。

### 5.7 共享本地元数据层

刷题管理的基础是一层独立的本地元数据层，存放于 `data/problemmeta.db`，并以 UUID 连接名管理数据库连接，与既有收藏仓库保持一致模式。数据层由 `ProblemMetaRepository`与 `ProblemMetaService`组成，核心数据结构为 `ProblemMeta`。初始表结构如下：
```sql
CREATE TABLE problem_meta (
    problem_url TEXT PRIMARY KEY,               -- 题目链接
    title TEXT,                                 -- 题目标题  
    note TEXT,                                  -- 笔记内容
    difficulty INTEGER DEFAULT 0,               -- 难度值
    task_status TEXT DEFAULT 'todo',            -- 任务状态
    priority INTEGER DEFAULT 0,                 -- 优先级，数值越大通常越优先
    deadline TEXT,                              -- 截止时间
    review_flag INTEGER DEFAULT 0,              -- 是否加入错题\复习列表
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP   -- 最近更新时间
);

CREATE TABLE tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,       -- 标签的自增主键
    name TEXT NOT NULL UNIQUE                   -- 标签名
);

CREATE TABLE problem_tags (
    problem_url TEXT NOT NULL,                  -- 题目链接
    tag_id INTEGER NOT NULL,                    -- 标签 ID
    PRIMARY KEY(problem_url, tag_id)            -- 用于标记题和标签的组合不能重复
);
```

任务状态以英文枚举存储（`todo` / `doing` / `done` / `redo`），在 UI 层映射为中文（未开始 / 进行中 / 已完成 / 待重做）。仓库提供 `upsertMeta`、`loadMeta`、`loadAllMeta`（按 `updated_at` 降序）、`setTags`、`statusCounts`、`tagCounts`、`reviewProblems`、`notesCount` 等方法，供上层看板、统计与错题本按需组合。

### 5.8 Notes 侧栏、任务看板与统计/错题本

Notes 侧栏在题目页工作区第三栏，包含任务状态、难度、标签、错题本勾选和笔记编辑，点击 Save 保存。  
任务看板用四栏展示未开始 / 进行中 / 已完成 / 待重做的题目，存储题目地址`problemUrl` 与标题。单击打开题目页，还实现了跨列拖拽改状态，顶部提供标签、难度、排序三个下拉框做前端过滤与排序，不改动 SQL。  
统计与错题本左侧用拉伸条形展示任务状态分布、标签 Top 8、笔记数与收藏题数；右侧列出 错题，支持按标签、难度前端筛选，标题实时显示「显示数/总数」。  

### 5.9 间隔重复复习（SRS）

在错题本之上引入类 Anki 的间隔重复闭环，全程本地。评分采用简化 SM-2 算法，分 Again / Hard / Good / Easy 四档：Again 间隔归零、ease 下调；Good 首次间隔为 1 天、其后按 `interval × ease / 100` 递增；Hard/Easy 在此基础上调整系数与 ease。到期判定统一用本地时区的 ISO 时间字符串比较。评分更新有意不改 `updated_at`，避免复习把题目顶到看板与统计列表最前。  
复习页左侧列出到期题目，右侧只读展示标题、难度、标签、笔记，配四个评分按钮与「打开题目」。勾选错题本即触发首次排程，下次复习设为当天。

### 5.10 配置与运行时数据

服务地址可以通过`appstate.toml`配置，包括：openjudge网站地址`openjudge_base_url`、测试服务器地址`judger_base_url`、邮件验证服务地址`email_verify_url`，未配置时使用内置默认值，`MainWindow` 启动时读取到客户端。为消除配置带结尾 `/` 时拼出 `//path` 的问题，网络层在存入前统一 `trimmed()` 并去掉结尾斜杠。以上均可在 Settings 页配置，保存后即时生效、无需重启。

为了避免把数据库、缓存、`config.toml` / `appstate.toml`、日志都写在项目目录，Windows 安装到 `Program Files` 时该目录只读会导致写入失败的问题，使用统一路径工具 `AppPaths`，把数据与缓存迁到 `QStandardPaths::GenericDataLocation` 下的 `oj-client` 目录，并在首次启动时一次性迁移关键库与 toml。

### 5.11 提醒与系统托盘

应用会检查due soon列表中的作业截止时间，在开启闹钟时立即检查一次，之后每到整点检查一次，当比赛距截止剩余 1/2/3 小时时各提醒一次，触发时显示托盘通知并播放指定的本地音频。为了保证铃声功能能够正常工作，应用要保持运行状态，故设置关闭主窗口默认最小化到托盘，然后再从托盘恢复或退出。


## 六、局限与风险

- **解析依赖页面结构**：OpenJudge 以网页为主、无公开结构化 API，页面结构变化可能导致解析失效。应对方式是集中管理解析规则、对关键字段缺失容错，并保留「在浏览器中打开」作为后备入口。

- **AI 工具调用依赖服务能力**：工具调用能力取决于所接入 OpenAI 兼容服务是否正确支持 `/responses`，否则会降级到 `/chat/completions`，退回到最基础的对话。

- **最小化到托盘可能不会有提示**：在使用windows测试阶段，最小化到托盘伴随着一次系统提示。若缺失系统提示，用户可能不知道系统已经最小化到托盘，再次启动程序可能导致多个程序进程同时启动占用内存。

- **oj-judger可靠性不足**：当前的测试程序运行在Clab云主机上，其性能有限故对并发请求响应不良，且由于无法完全确保测试环境的安全性，该功能仅限小范围使用难以推广。

- **代码编辑功能过于简单**：当前代码框只包含语法高亮和缩进，距离成熟代码编辑器的体验相差很远，如何权衡完整的功能与轻量化的实现，需要进一步考虑。


## 七、总结与展望

通过完成这个项目，我们对桌面应用的整体组织结构、页面之间的协作关系以及数据存储有了更深入的理解，也体验了将网页功能迁移到本地客户端的过程。在项目的开发中，我们更清楚地认识到了模块化、层次化设计和协同的重要性，极大提升了开发/维护/debug效率。

后续应对当前的不足和风险逐步修补，此外还可以继续向“更懂刷题”的方向发展，比如增加更细致的题目管理方式、更强的复习提醒、更丰富的统计视图，以及更智能的辅助能力，让它从一个好用的 OJ 客户端逐步演化成一个更完整的个人算法学习平台。

## 八、 分工

田昊阳：
- 视频录制，报告撰写与修改  
- oj-judger、oj-server两个拓展程序，实现登录验证、代码测试功能
- UI / Service / Repository / Network / Parser 的全流程结构。
- 前端界面，图标，主题，铃声设计
- 辅助功能包括：闹钟提醒，ai对话，题目收藏，简化代码编辑器

盛适：
- 报告撰写
- 添加优化了本地刷题管理机制，包括：收藏系统与 JSON 导入导出；共享本地元数据层；任务看板与统计/错题本；间隔重复复习机制（SRS）
- 对parser进行工程化优化

雷涛诚：
- 项目功能测试
- 项目报告辅助撰写