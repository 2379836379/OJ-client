#include "ui/pages/statspage.h"
#include "ui/lightmodeiconhelper.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
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
}

StatsPage::StatsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("statsTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("Statistics", topFrame);
    titleLabel->setObjectName("statsTitleLabel");
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("statsTopActionButton");
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("statsTopActionButton");
    auto *refreshButton = new QPushButton("Refresh", topFrame);
    refreshButton->setObjectName("statsTopActionButton");

    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(homeButton);
    topLayout->addWidget(themeButton);
    topLayout->addWidget(refreshButton);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    auto *summaryFrame = new QFrame(this);
    summaryFrame->setObjectName("statsSummaryFrame");
    auto *summaryOuter = new QVBoxLayout(summaryFrame);
    summaryOuter->setContentsMargins(20, 18, 20, 18);
    summaryOuter->setSpacing(12);
    auto *summaryTitle = new QLabel("Overview", summaryFrame);
    summaryTitle->setObjectName("statsSectionLabel");
    m_summaryLayout = new QVBoxLayout();
    m_summaryLayout->setContentsMargins(0, 0, 0, 0);
    m_summaryLayout->setSpacing(8);
    summaryOuter->addWidget(summaryTitle);
    summaryOuter->addLayout(m_summaryLayout);
    summaryOuter->addStretch();

    auto *reviewFrame = new QFrame(this);
    reviewFrame->setObjectName("statsReviewFrame");
    auto *reviewOuter = new QVBoxLayout(reviewFrame);
    reviewOuter->setContentsMargins(20, 18, 20, 18);
    reviewOuter->setSpacing(12);
    m_reviewLabel = new QLabel("错题本", reviewFrame);
    m_reviewLabel->setObjectName("statsSectionLabel");

    auto *reviewFilterLayout = new QHBoxLayout();
    reviewFilterLayout->setSpacing(8);
    auto *reviewTagLabel = new QLabel("标签", reviewFrame);
    reviewTagLabel->setObjectName("statFilterLabel");
    m_reviewTagFilter = new QComboBox(reviewFrame);
    m_reviewTagFilter->setObjectName("statFilterCombo");
    m_reviewTagFilter->addItem("全部标签", QString());
    auto *reviewDifficultyLabel = new QLabel("难度", reviewFrame);
    reviewDifficultyLabel->setObjectName("statFilterLabel");
    m_reviewDifficultyFilter = new QComboBox(reviewFrame);
    m_reviewDifficultyFilter->setObjectName("statFilterCombo");
    m_reviewDifficultyFilter->addItem("全部难度", -1);
    m_reviewDifficultyFilter->addItem("未设置", 0);
    for (int level = 1; level <= 5; ++level) {
        m_reviewDifficultyFilter->addItem(QString::number(level), level);
    }
    reviewFilterLayout->addWidget(reviewTagLabel);
    reviewFilterLayout->addWidget(m_reviewTagFilter);
    reviewFilterLayout->addSpacing(8);
    reviewFilterLayout->addWidget(reviewDifficultyLabel);
    reviewFilterLayout->addWidget(m_reviewDifficultyFilter);
    reviewFilterLayout->addStretch();

    m_reviewList = new QListWidget(reviewFrame);
    m_reviewList->setObjectName("statsReviewList");
    m_reviewList->setMinimumHeight(360);
    reviewOuter->addWidget(m_reviewLabel);
    reviewOuter->addLayout(reviewFilterLayout);
    reviewOuter->addWidget(m_reviewList, 1);

    bottomLayout->addWidget(summaryFrame, 1);
    bottomLayout->addWidget(reviewFrame, 1);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    refreshButton->setToolTip("Refresh");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(refreshButton, "refresh.svg");

    setStyleSheet(
        "StatsPage { background: #f3f1eb; }"
        "#statsTopFrame, #statsSummaryFrame, #statsReviewFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#statsTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#statsSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#statsTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#statRowLabel { color: #2f3a33; }"
        "#statRowValue { color: #5e675f; }"
        "#statBarHolder { background: transparent; }"
        "#statBarFill { background: #7fb8a6; border-radius: 4px; }"
        "#statBarRest { background: #e6e1d7; border-radius: 4px; }"
        "#statsReviewList {"
        "  background: transparent;"
        "  border: none;"
        "  outline: none;"
        "  color: #1f2328;"
        "}"
        "#statsReviewList::item {"
        "  padding: 10px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#statsReviewList::item:selected { background: #dcefea; color: #12343b; }"
        "#statsReviewList::item:hover { background: #eef4ef; }"
        "#statFilterLabel { color: #2f3a33; }"
        "#statFilterCombo {"
        "  padding: 4px 8px;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 8px;"
        "  background: #fbfaf7;"
        "  color: #2f3a33;"
        "  min-width: 90px;"
        "}"
        "#statFilterCombo QAbstractItemView {"
        "  border: 1px solid #ded8cc;"
        "  background: #fbfaf7;"
        "  color: #2f3a33;"
        "  selection-background-color: #dcefea;"
        "  selection-color: #12343b;"
        "}");

    connect(homeButton, &QPushButton::clicked, this, &StatsPage::homeRequested);
    connect(refreshButton, &QPushButton::clicked, this, &StatsPage::refreshRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_reviewList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item == nullptr) {
            return;
        }
        emit problemSelected(item->data(ProblemTitleRole).toString(),
                             item->data(ProblemUrlRole).toString());
    });
    connect(m_reviewTagFilter, &QComboBox::currentIndexChanged, this,
            [this](int) { renderReview(); });
    connect(m_reviewDifficultyFilter, &QComboBox::currentIndexChanged, this,
            [this](int) { renderReview(); });
}

void StatsPage::clearLayout(QVBoxLayout *layout)
{
    if (layout == nullptr) {
        return;
    }
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void StatsPage::addStatRow(QVBoxLayout *layout,
                           const QString &label,
                           int value,
                           int maxValue)
{
    auto *row = new QWidget();
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    auto *name = new QLabel(label, row);
    name->setObjectName("statRowLabel");
    name->setFixedWidth(150);

    auto *barHolder = new QFrame(row);
    barHolder->setObjectName("statBarHolder");
    barHolder->setFixedHeight(16);
    auto *barLayout = new QHBoxLayout(barHolder);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(0);
    auto *fill = new QFrame(barHolder);
    fill->setObjectName("statBarFill");
    auto *rest = new QFrame(barHolder);
    rest->setObjectName("statBarRest");
    barLayout->addWidget(fill);
    barLayout->addWidget(rest);

    const int clampedValue = qMax(0, value);
    const int effectiveMax = qMax(maxValue, 1);
    barLayout->setStretch(0, clampedValue);
    barLayout->setStretch(1, qMax(0, effectiveMax - clampedValue));
    if (clampedValue == 0) {
        fill->hide();
    }

    auto *valueLabel = new QLabel(QString::number(value), row);
    valueLabel->setObjectName("statRowValue");
    valueLabel->setFixedWidth(48);

    rowLayout->addWidget(name);
    rowLayout->addWidget(barHolder, 1);
    rowLayout->addWidget(valueLabel);
    layout->addWidget(row);
}

void StatsPage::showStats(const QHash<QString, int> &statusCounts,
                          const QHash<QString, int> &tagCounts,
                          int notesCount,
                          const QList<FavoriteFolderInfo> &folders,
                          const QList<ProblemMeta> &reviewProblems)
{
    clearLayout(m_summaryLayout);

    auto *statusHeader = new QLabel("任务状态");
    statusHeader->setObjectName("statRowLabel");
    m_summaryLayout->addWidget(statusHeader);

    int maxStatus = 1;
    for (const StatusInfo &info : statusInfos()) {
        maxStatus = qMax(maxStatus, statusCounts.value(info.key, 0));
    }
    for (const StatusInfo &info : statusInfos()) {
        addStatRow(m_summaryLayout, info.label, statusCounts.value(info.key, 0), maxStatus);
    }

    auto *tagHeader = new QLabel("标签分布 (Top 8)");
    tagHeader->setObjectName("statRowLabel");
    m_summaryLayout->addWidget(tagHeader);

    QList<QPair<QString, int>> sortedTags;
    for (auto it = tagCounts.constBegin(); it != tagCounts.constEnd(); ++it) {
        sortedTags.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedTags.begin(), sortedTags.end(),
              [](const QPair<QString, int> &lhs, const QPair<QString, int> &rhs) {
                  if (lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                  }
                  return lhs.first < rhs.first;
              });
    if (sortedTags.isEmpty()) {
        auto *empty = new QLabel("暂无标签");
        empty->setObjectName("statRowValue");
        m_summaryLayout->addWidget(empty);
    } else {
        const int maxTag = sortedTags.first().second;
        const int limit = qMin(8, static_cast<int>(sortedTags.size()));
        for (int i = 0; i < limit; ++i) {
            addStatRow(m_summaryLayout, sortedTags.at(i).first, sortedTags.at(i).second, maxTag);
        }
    }

    int favoriteTotal = 0;
    for (const FavoriteFolderInfo &folder : folders) {
        favoriteTotal += folder.problemCount;
    }

    auto *otherHeader = new QLabel("其它");
    otherHeader->setObjectName("statRowLabel");
    m_summaryLayout->addWidget(otherHeader);
    const int otherMax = qMax(1, qMax(notesCount, favoriteTotal));
    addStatRow(m_summaryLayout, "笔记数量", notesCount, otherMax);
    addStatRow(m_summaryLayout, "收藏题数", favoriteTotal, otherMax);

    m_reviewProblems = reviewProblems;
    rebuildReviewTagOptions();
    renderReview();
}

void StatsPage::rebuildReviewTagOptions()
{
    if (m_reviewTagFilter == nullptr) {
        return;
    }

    const QString current = m_reviewTagFilter->currentData().toString();
    QStringList tags;
    for (const ProblemMeta &meta : m_reviewProblems) {
        for (const QString &tag : meta.tags) {
            if (!tags.contains(tag, Qt::CaseInsensitive)) {
                tags << tag;
            }
        }
    }
    tags.sort(Qt::CaseInsensitive);

    const QSignalBlocker blocker(m_reviewTagFilter);
    m_reviewTagFilter->clear();
    m_reviewTagFilter->addItem("全部标签", QString());
    for (const QString &tag : tags) {
        m_reviewTagFilter->addItem(tag, tag);
    }
    const int index = m_reviewTagFilter->findData(current);
    m_reviewTagFilter->setCurrentIndex(index >= 0 ? index : 0);
}

void StatsPage::renderReview()
{
    if (m_reviewList == nullptr) {
        return;
    }

    const QString tagFilter =
        m_reviewTagFilter != nullptr ? m_reviewTagFilter->currentData().toString() : QString();
    const int difficultyFilter =
        m_reviewDifficultyFilter != nullptr ? m_reviewDifficultyFilter->currentData().toInt() : -1;

    m_reviewList->clear();
    int shown = 0;
    for (const ProblemMeta &meta : m_reviewProblems) {
        if (!tagFilter.isEmpty() && !meta.tags.contains(tagFilter, Qt::CaseInsensitive)) {
            continue;
        }
        if (difficultyFilter >= 0 && meta.difficulty != difficultyFilter) {
            continue;
        }
        const QString displayText = meta.title.isEmpty() ? meta.problemUrl : meta.title;
        auto *item = new QListWidgetItem(displayText, m_reviewList);
        item->setData(ProblemUrlRole, meta.problemUrl);
        item->setData(ProblemTitleRole, meta.title);
        if (!meta.tags.isEmpty()) {
            item->setToolTip(meta.tags.join(", "));
        }
        ++shown;
    }
    m_reviewLabel->setText(QString("错题本 (%1/%2)").arg(shown).arg(m_reviewProblems.size()));
}

void StatsPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "StatsPage { background: #000000; }"
        "#statsTopFrame, #statsSummaryFrame, #statsReviewFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#statsTitleLabel, #statsSectionLabel { color: #d9e1e8; }"
        "#statsTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#statsTopActionButton:hover { background: #26313c; }"
        "#statRowLabel { color: #d9e1e8; }"
        "#statRowValue { color: #9ba8b6; }"
        "#statBarFill { background: #3d8d77; }"
        "#statBarRest { background: #2c3844; }"
        "#statsReviewList { color: #e8edf2; }"
        "#statsReviewList::item:selected { background: #234257; color: #eff8ff; }"
        "#statsReviewList::item:hover { background: #26313c; }"
        "#statFilterLabel { color: #d9e1e8; }"
        "#statFilterCombo {"
        "  border: 1px solid #2c3844;"
        "  background: #1b232c;"
        "  color: #e8edf2;"
        "}"
        "#statFilterCombo QAbstractItemView {"
        "  border: 1px solid #2c3844;"
        "  background: #1b232c;"
        "  color: #e8edf2;"
        "  selection-background-color: #234257;"
        "  selection-color: #eff8ff;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}