#include "ui/pages/taskboardpage.h"
#include "ui/lightmodeiconhelper.h"

#include <QComboBox>
#include <QDropEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
constexpr int ProblemUrlRole = Qt::UserRole;
constexpr int ProblemTitleRole = Qt::UserRole + 1;

struct StatusInfo
{
    QString key;
    QString label;
};

const QList<StatusInfo> &statusInfos()
{
    static const QList<StatusInfo> infos = {
        {"todo", "未开始"},
        {"doing", "进行中"},
        {"done", "已完成"},
        {"redo", "待重做"}};
    return infos;
}

QWidget *makeColumn(const QString &title,
                    const QString &statusKey,
                    StatusListWidget **listOut,
                    QWidget *parent)
{
    auto *column = new QFrame(parent);
    column->setObjectName("taskColumnFrame");
    auto *layout = new QVBoxLayout(column);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *label = new QLabel(title, column);
    label->setObjectName("taskColumnLabel");

    auto *list = new StatusListWidget(statusKey, column);
    list->setObjectName("taskListWidget");
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    list->setMinimumHeight(360);

    layout->addWidget(label);
    layout->addWidget(list, 1);
    *listOut = list;
    return column;
}
}

StatusListWidget::StatusListWidget(const QString &status, QWidget *parent)
    : QListWidget(parent)
    , m_status(status)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
}

QString StatusListWidget::status() const
{
    return m_status;
}

void StatusListWidget::dropEvent(QDropEvent *event)
{
    auto *source = qobject_cast<StatusListWidget *>(event->source());
    if (source != nullptr) {
        QListWidgetItem *item = source->currentItem();
        if (item != nullptr && source != this) {
            emit itemDropped(item->data(ProblemUrlRole).toString(), m_status);
        }
        // Persistence + refresh is handled by the page repaint, so prevent Qt
        // from physically moving the row (which would fight the repaint).
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
        return;
    }
    event->ignore();
}

TaskBoardPage::TaskBoardPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("taskTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("Task Board", topFrame);
    titleLabel->setObjectName("taskTitleLabel");
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("taskTopActionButton");
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("taskTopActionButton");
    auto *refreshButton = new QPushButton("Refresh", topFrame);
    refreshButton->setObjectName("taskTopActionButton");

    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(homeButton);
    topLayout->addWidget(themeButton);
    topLayout->addWidget(refreshButton);

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("taskContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(12);

    m_statusLabel = new QLabel(contentFrame);
    m_statusLabel->setObjectName("taskStatusLabel");
    m_statusLabel->setWordWrap(true);

    auto *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(8);
    auto *tagFilterLabel = new QLabel("标签", contentFrame);
    tagFilterLabel->setObjectName("taskFilterLabel");
    m_tagFilterCombo = new QComboBox(contentFrame);
    m_tagFilterCombo->setObjectName("taskFilterCombo");
    m_tagFilterCombo->addItem("全部标签", QString());
    auto *difficultyFilterLabel = new QLabel("难度", contentFrame);
    difficultyFilterLabel->setObjectName("taskFilterLabel");
    m_difficultyFilterCombo = new QComboBox(contentFrame);
    m_difficultyFilterCombo->setObjectName("taskFilterCombo");
    m_difficultyFilterCombo->addItem("全部难度", -1);
    m_difficultyFilterCombo->addItem("未设置", 0);
    for (int level = 1; level <= 5; ++level) {
        m_difficultyFilterCombo->addItem(QString::number(level), level);
    }
    auto *sortLabel = new QLabel("排序", contentFrame);
    sortLabel->setObjectName("taskFilterLabel");
    m_sortCombo = new QComboBox(contentFrame);
    m_sortCombo->setObjectName("taskFilterCombo");
    m_sortCombo->addItem("最近更新", "updated");
    m_sortCombo->addItem("难度高→低", "difficulty");
    m_sortCombo->addItem("标题", "title");

    filterLayout->addWidget(tagFilterLabel);
    filterLayout->addWidget(m_tagFilterCombo);
    filterLayout->addSpacing(8);
    filterLayout->addWidget(difficultyFilterLabel);
    filterLayout->addWidget(m_difficultyFilterCombo);
    filterLayout->addSpacing(8);
    filterLayout->addWidget(sortLabel);
    filterLayout->addWidget(m_sortCombo);
    filterLayout->addStretch();

    auto *columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(14);
    columnsLayout->addWidget(makeColumn("未开始", "todo", &m_todoList, contentFrame), 1);
    columnsLayout->addWidget(makeColumn("进行中", "doing", &m_doingList, contentFrame), 1);
    columnsLayout->addWidget(makeColumn("已完成", "done", &m_doneList, contentFrame), 1);
    columnsLayout->addWidget(makeColumn("待重做", "redo", &m_redoList, contentFrame), 1);

    contentLayout->addWidget(m_statusLabel);
    contentLayout->addLayout(filterLayout);
    contentLayout->addLayout(columnsLayout, 1);

    layout->addWidget(topFrame);
    layout->addWidget(contentFrame, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    refreshButton->setToolTip("Refresh");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(refreshButton, "refresh.svg");

    setStyleSheet(
        "TaskBoardPage { background: #f3f1eb; }"
        "#taskTopFrame, #taskContentFrame, #taskColumnFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#taskTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#taskColumnLabel {"
        "  font-size: 15px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#taskTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#taskTopActionButton:hover { background: transparent; }"
        "#taskStatusLabel { color: #7a4b36; min-height: 18px; }"
        "#taskListWidget {"
        "  background: transparent;"
        "  border: none;"
        "  outline: none;"
        "}"
        "#taskListWidget::item {"
        "  padding: 10px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#taskListWidget::item:selected { background: #dcefea; color: #12343b; }"
        "#taskListWidget::item:hover { background: #eef4ef; }"
        "#taskFilterLabel { color: #2f3a33; }"
        "#taskFilterCombo {"
        "  padding: 4px 8px;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 8px;"
        "  background: #fbfaf7;"
        "  color: #2f3a33;"
        "  min-width: 90px;"
        "}"
        "#taskFilterCombo QAbstractItemView {"
        "  border: 1px solid #ded8cc;"
        "  background: #fbfaf7;"
        "  color: #2f3a33;"
        "  selection-background-color: #dcefea;"
        "  selection-color: #12343b;"
        "}");

    const QList<StatusListWidget *> lists = {m_todoList, m_doingList, m_doneList, m_redoList};
    for (StatusListWidget *list : lists) {
        connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
            if (item == nullptr) {
                return;
            }
            emit problemSelected(item->data(ProblemTitleRole).toString(),
                                 item->data(ProblemUrlRole).toString());
        });
        connect(list, &QListWidget::customContextMenuRequested, this,
                [this, list](const QPoint &pos) {
                    showStatusMenu(list, pos);
                });
        connect(list, &StatusListWidget::itemDropped, this,
                [this](const QString &url, const QString &targetStatus) {
                    if (!url.isEmpty()) {
                        emit statusChangeRequested(url, targetStatus);
                    }
                });
    }

    connect(m_tagFilterCombo, &QComboBox::currentIndexChanged, this,
            [this](int) { renderTasks(); });
    connect(m_difficultyFilterCombo, &QComboBox::currentIndexChanged, this,
            [this](int) { renderTasks(); });
    connect(m_sortCombo, &QComboBox::currentIndexChanged, this,
            [this](int) { renderTasks(); });

    connect(homeButton, &QPushButton::clicked, this, &TaskBoardPage::homeRequested);
    connect(refreshButton, &QPushButton::clicked, this, &TaskBoardPage::refreshRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
}

StatusListWidget *TaskBoardPage::listForStatus(const QString &status) const
{
    if (status == "doing") {
        return m_doingList;
    }
    if (status == "done") {
        return m_doneList;
    }
    if (status == "redo") {
        return m_redoList;
    }
    return m_todoList;
}

void TaskBoardPage::showStatusMenu(QListWidget *list, const QPoint &pos)
{
    if (list == nullptr) {
        return;
    }
    QListWidgetItem *item = list->itemAt(pos);
    if (item == nullptr) {
        return;
    }

    const QString url = item->data(ProblemUrlRole).toString();
    QMenu menu(this);
    for (const StatusInfo &info : statusInfos()) {
        QAction *action = menu.addAction(QString("移动到: %1").arg(info.label));
        const QString statusKey = info.key;
        connect(action, &QAction::triggered, this, [this, url, statusKey]() {
            emit statusChangeRequested(url, statusKey);
        });
    }
    menu.exec(list->mapToGlobal(pos));
}

void TaskBoardPage::showTasks(const QList<ProblemMeta> &allMeta)
{
    m_allMeta = allMeta;
    rebuildTagFilterOptions();
    renderTasks();
}

void TaskBoardPage::rebuildTagFilterOptions()
{
    if (m_tagFilterCombo == nullptr) {
        return;
    }

    const QString current = m_tagFilterCombo->currentData().toString();
    QStringList tags;
    for (const ProblemMeta &meta : m_allMeta) {
        for (const QString &tag : meta.tags) {
            if (!tags.contains(tag, Qt::CaseInsensitive)) {
                tags << tag;
            }
        }
    }
    tags.sort(Qt::CaseInsensitive);

    const QSignalBlocker blocker(m_tagFilterCombo);
    m_tagFilterCombo->clear();
    m_tagFilterCombo->addItem("全部标签", QString());
    for (const QString &tag : tags) {
        m_tagFilterCombo->addItem(tag, tag);
    }
    const int index = m_tagFilterCombo->findData(current);
    m_tagFilterCombo->setCurrentIndex(index >= 0 ? index : 0);
}

void TaskBoardPage::renderTasks()
{
    const QList<StatusListWidget *> lists = {m_todoList, m_doingList, m_doneList, m_redoList};
    for (StatusListWidget *list : lists) {
        list->clear();
    }

    const QString tagFilter =
        m_tagFilterCombo != nullptr ? m_tagFilterCombo->currentData().toString() : QString();
    const int difficultyFilter =
        m_difficultyFilterCombo != nullptr ? m_difficultyFilterCombo->currentData().toInt() : -1;
    const QString sortKey =
        m_sortCombo != nullptr ? m_sortCombo->currentData().toString() : QStringLiteral("updated");

    QList<ProblemMeta> filtered;
    for (const ProblemMeta &meta : m_allMeta) {
        if (!tagFilter.isEmpty() && !meta.tags.contains(tagFilter, Qt::CaseInsensitive)) {
            continue;
        }
        if (difficultyFilter >= 0 && meta.difficulty != difficultyFilter) {
            continue;
        }
        filtered << meta;
    }

    if (sortKey == QStringLiteral("difficulty")) {
        std::stable_sort(filtered.begin(), filtered.end(),
                         [](const ProblemMeta &a, const ProblemMeta &b) {
                             return a.difficulty > b.difficulty;
                         });
    } else if (sortKey == QStringLiteral("title")) {
        std::stable_sort(filtered.begin(), filtered.end(),
                         [](const ProblemMeta &a, const ProblemMeta &b) {
                             return a.title.localeAwareCompare(b.title) < 0;
                         });
    }

    for (const ProblemMeta &meta : filtered) {
        StatusListWidget *list = listForStatus(meta.taskStatus);
        const QString displayText = meta.title.isEmpty() ? meta.problemUrl : meta.title;
        auto *item = new QListWidgetItem(displayText, list);
        item->setData(ProblemUrlRole, meta.problemUrl);
        item->setData(ProblemTitleRole, meta.title);
        if (!meta.tags.isEmpty()) {
            item->setToolTip(meta.tags.join(", "));
        }
    }

    if (m_allMeta.isEmpty()) {
        m_statusLabel->setText("还没有任何题目记录。在题目页的 Notes 面板保存后会出现在这里。");
    } else if (filtered.isEmpty()) {
        m_statusLabel->setText("没有符合筛选条件的题目。");
    } else {
        m_statusLabel->setText(
            QString("共 %1 道题目记录。可拖拽题目到其它列改变状态，或右键移动。")
                .arg(filtered.size()));
    }
}

void TaskBoardPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "TaskBoardPage { background: #000000; }"
        "#taskTopFrame, #taskContentFrame, #taskColumnFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#taskTitleLabel, #taskColumnLabel { color: #d9e1e8; }"
        "#taskTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#taskTopActionButton:hover { background: #26313c; }"
        "#taskStatusLabel { color: #f0b48a; }"
        "#taskListWidget { color: #e8edf2; }"
        "#taskListWidget::item:selected { background: #234257; color: #eff8ff; }"
        "#taskListWidget::item:hover { background: #26313c; }"
        "#taskFilterLabel { color: #d9e1e8; }"
        "#taskFilterCombo {"
        "  border: 1px solid #2c3844;"
        "  background: #1b232c;"
        "  color: #e8edf2;"
        "}"
        "#taskFilterCombo QAbstractItemView {"
        "  border: 1px solid #2c3844;"
        "  background: #1b232c;"
        "  color: #e8edf2;"
        "  selection-background-color: #234257;"
        "  selection-color: #eff8ff;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}