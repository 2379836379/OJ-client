# oj-client 演进计划（PLAN）

本计划覆盖三条主线，均建立在本轮已完成的本地元数据层（`problemmeta.db`）之上，
尽量纯增量、不改提交/判题/AI/收藏既有核心逻辑。

- A. 工程加固（基础设施）
- B. 任务看板拖拽 + 看板/错题本筛选排序
- C. 间隔重复复习闭环（SRS）

实施顺序建议：A1（URL 规整）→ B → C → A2（数据落用户目录）→ A3（Settings 页）。
理由：A1 极低风险先清账；B/C 价值高且只依赖现有元数据层；A2 有迁移风险放在功能稳定后单独做；A3 体量大放最后。

---

## A. 工程加固

### A1. base URL 规整（低成本 / 低风险）

**目标**：消除 `judger_base_url` / `openjudge_base_url` / `email_verify_url` 带结尾 `/` 时拼出 `//path` 的问题。

**改动点**：
- `src/network/openjudgeclient.cpp`：`setBaseUrl` / `setJudgerBaseUrl` 存入前 `trimmed()` 并去掉结尾 `/`；拼接处保持 `base + "/judge"` 形式。
- `src/ui/mainwindow.cpp`：`email_verify_url` 注入处同样规整（或在 `EmailVerifyService::setBaseUrl` 内处理）。

**验收**：配置写成 `http://host:18080/` 与 `http://host:18080` 行为一致；默认值不受影响。

**风险**：极低。

---

### A2. 数据落用户目录（中成本 / 中风险，需迁移）

**目标**：把 `problemmeta.db`、收藏库、各类缓存、`appstate.toml` 从"项目/安装目录"迁到
`QStandardPaths::writableLocation(AppDataLocation)`，解决 Windows 安装目录无写权限问题（README 已知限制）。

**改动点**：
- 新增统一路径工具（如 `src/config/apppaths.h/.cpp`），提供 `dataDir()` / `cacheDir()`。
- 各 repository 的 `projectRootDir()/databasePath()` 改为调用统一工具。
- `appconfig.cpp` 的 `appStateOutputPath()` / `configOutputPath()` 改为用户目录优先。

**迁移策略（开放问题，见文末）**：首次启动若用户目录无数据但项目目录有，则一次性拷贝。

**验收**：全新环境能创建并读写；旧数据可被迁移（若采纳迁移）；安装到 `Program Files` 后正常。

**风险**：路径改动牵涉多模块；迁移逻辑属 fallback，需确认并记入 DEVLOG。

---

### A3. Settings 页（中成本 / 低风险）

**目标**：把 `appstate.toml` 的三个服务地址、闹钟开关、铃声路径，以及 AI 配置做成 UI。

**改动点**：复用 `AiConfigPage` 模式新增 `SettingsPage`，调用已有 `AppConfig::save*`；首页工具栏加入口。

**验收**：UI 修改能写回 toml 并即时生效（地址注入到 client/service）。

**风险**：低，纯增量页面。

---

## B. 看板拖拽 + 筛选排序

### B1. 看板跨列拖拽改状态（中成本）

**目标**：四个状态列之间拖拽题目即改 `task_status`，替代/补充现有右键菜单。

**设计**：
- 四个 `QListWidget` 设置 `setDragEnabled(true)`、`setAcceptDrops(true)`、
  `setDropIndicatorShown(true)`、`setDragDropMode(QAbstractItemView::DragDrop)`、
  `setDefaultDropAction(Qt::MoveAction)`。
- 以事件过滤器或轻量子类捕获 drop：得到目标列对应 status，对被拖项 `emit statusChangeRequested(url, newStatus)`，
  随后 `showTasks(loadAllMeta())` 重绘（沿用现有保存链路，避免手动维护列内顺序）。

**改动点**：`src/ui/pages/taskboardpage.h/.cpp`（主要）；`mainwindow.cpp` 无需改动（信号已接）。

**验收**：拖拽后状态持久化、刷新后仍在新列；右键菜单保留可用。

**风险**：拖拽事件处理需小心 item 所有权；保持"拖完整体重绘"可规避顺序/重复项问题。

---

### B2. 看板 / 错题本筛选排序（低-中成本）

**目标**：按标签、难度过滤，按更新时间/难度/deadline 排序。

**设计**：数据已在内存（`loadAllMeta()` / `reviewProblems()`），在页面前端过滤，不动 SQL。
顶部加 `QComboBox`（标签）、`QComboBox`（难度）、排序 `QComboBox`。

**改动点**：`taskboardpage.*`、`statspage.*`（错题本列表区）。

**验收**：切换筛选条件实时更新列表；空结果有提示。

**风险**：低。

---

## C. 间隔重复复习闭环（SRS）

### 目标

基于错题本（`review_flag=1`）做类 Anki 的间隔重复：每道复习题维护下次复习时间，
首页提示"今日待复习 N 题"，复习时按掌握度评分并据简化 SM-2 更新间隔。纯本地、零网络。

### 数据结构（`problem_meta` 增列，向后兼容）

```sql
ALTER TABLE problem_meta ADD COLUMN review_interval INTEGER DEFAULT 0;   -- 当前间隔(天)
ALTER TABLE problem_meta ADD COLUMN review_ease     INTEGER DEFAULT 250; -- ease factor x100
ALTER TABLE problem_meta ADD COLUMN next_review_at  TEXT;                -- ISO 时间, 空=未排程
ALTER TABLE problem_meta ADD COLUMN last_reviewed_at TEXT;
ALTER TABLE problem_meta ADD COLUMN review_count    INTEGER DEFAULT 0;
```

迁移用 `PRAGMA table_info(problem_meta)` 检测列是否存在，缺失才 `ALTER TABLE`（避免重复执行报错）。

### 评分与算法（简化 SM-2）

四档：Again / Hard / Good / Easy。
- Again：interval=0（当天再来），ease 下调。
- Good：首次 interval=1，其后 `interval = round(interval * ease/100)`。
- Hard/Easy 在 Good 基础上下/上调系数并调整 ease。
- `next_review_at = now + interval 天`；勾选错题本即首次排程（next_review_at=今天）。

### UI

- 新增 `ReviewPage`：顶部统计"今日待复习 N"；列表为 due 题目；选中后展示笔记/标签 + 四个评分按钮 + "打开题目"。
- `HomePage` 新增入口信号 `reviewRequested()` 与按钮（展开 + 折叠）。
- `ProblemMetaRepository` 新增：`dueReviewProblems(nowIso)`、`updateReviewSchedule(url, interval, ease, nextAt, lastAt, count)`；`ProblemMetaService` 同步薄封装。

### 改动点

- `repository/meta/problemmetarepository.*`、`service/meta/problemmetaservice.*`
- 新增 `ui/pages/reviewpage.*`、`homepage.*`、`mainwindow.*`、`CMakeLists.txt`

### 验收

- 勾选"加入错题本"后题目进入复习排程；
- 评分后 `next_review_at` 按算法推进，下次到期才再次出现；
- 首页计数与 due 列表一致；旧库升级不丢数据。

### 风险

- schema 迁移需幂等；
- 时间统一用本地时区 ISO 字符串，避免比较歧义。

---

## 待你拍板的开放问题

1. **A2 数据迁移**：是否要"首次启动自动从项目目录拷贝旧数据到用户目录"？（涉及 fallback 逻辑，按约定需确认并记 DEVLOG）还是只切换路径、不迁移旧数据？
2. **B1 拖拽 vs 右键**：拖拽是否完全替代右键菜单，还是两者并存（我倾向并存）？
3. **C SRS 落点**：复习入口做成独立 `ReviewPage`（推荐），还是并入现有 `StatsPage` 错题本区？
4. **整体节奏**：是否按 A1 → B → C → A2 → A3 推进；先做哪一个我就先实现哪一个，每阶段做完同步更新本 PLAN 与根目录 DEVLOG。

---

## 进度跟踪

- [x] A1 base URL 规整
- [x] B1 看板拖拽
- [x] B2a 看板筛选排序（标签/难度/排序）
- [x] B2b 错题本筛选排序（statspage）
- [x] C SRS 间隔重复复习（首页入口仅按钮，未做实时计数 N，见 DEVLOG）
- [x] A2 数据落用户目录（DB/缓存/toml/日志 → AppData，首启迁移关键库+toml，见 DEVLOG）
- [x] A3 Settings 页（仅做缺失的三个服务地址，保存即时生效；闹钟/铃声/AI 已有 UI 未重复，见 DEVLOG）
