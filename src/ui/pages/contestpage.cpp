#include "ui/pages/contestpage.h"
#include "ui/lightmodeiconhelper.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace
{
constexpr int ProblemTextRole = Qt::UserRole + 1;
constexpr int ProblemSolvedRole = Qt::UserRole + 2;

class ContestProblemDelegate final : public QStyledItemDelegate
{
public:
    explicit ContestProblemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        opt.text.clear();

        QStyle *style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        painter->save();

        const QRect contentRect = opt.rect.adjusted(16, 0, -16, 0);
        const QString title = index.data(ProblemTextRole).toString();
        const bool solved = index.data(ProblemSolvedRole).toBool();
        const QString statusText = solved ? QString("finished") : QString();
        const bool darkMode = opt.widget != nullptr
            && opt.widget->property("_darkMode").toBool();

        QColor titleColor = opt.palette.color(QPalette::Text);
        QColor statusColor = solved
            ? (darkMode ? QColor("#7CFF8A") : QColor("#23C552"))
            : opt.palette.color(QPalette::Mid);
        if (opt.state & QStyle::State_Selected) {
            titleColor = darkMode ? QColor("#eff8ff") : QColor("#12343b");
            if (solved) {
                statusColor = darkMode ? QColor("#9BFF8C") : QColor("#2DD85F");
            } else {
                statusColor = titleColor;
            }
        }

        const int statusSpacing = statusText.isEmpty() ? 0 : 16;
        const int statusWidth = statusText.isEmpty()
            ? 0
            : QFontMetrics(opt.font).horizontalAdvance(statusText);
        QRect titleRect = contentRect;
        if (statusWidth > 0) {
            titleRect.setRight(contentRect.right() - statusWidth - statusSpacing);
        }

        painter->setFont(opt.font);
        painter->setPen(titleColor);
        painter->drawText(titleRect,
                          Qt::AlignVCenter | Qt::AlignLeft,
                          QFontMetrics(opt.font).elidedText(title, Qt::ElideRight, titleRect.width()));

        if (!statusText.isEmpty()) {
            QFont statusFont = opt.font;
            statusFont.setBold(true);
            painter->setFont(statusFont);
            painter->setPen(statusColor);
            painter->drawText(contentRect,
                              Qt::AlignVCenter | Qt::AlignRight,
                              statusText);
        }

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 42));
        return size;
    }
};
}

ContestPage::ContestPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("contestTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    m_titleLabel = new QLabel("Contest Set", topFrame);
    m_titleLabel->setObjectName("contestTitleLabel");

    topLayout->addWidget(m_titleLabel, 1);
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("contestTopActionButton");
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("contestTopActionButton");
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);
    auto *refreshButton = new QPushButton("Refresh", topFrame);
    refreshButton->setObjectName("contestTopActionButton");
    topLayout->addWidget(refreshButton, 0, Qt::AlignRight);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("contestLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("contestToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);

    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("contestToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    m_collapsedToolsPanel->setObjectName("contestCollapsedToolsPanel");
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);

    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("contestToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("contestContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(14);

    auto *problemLabel = new QLabel("Problem List", contentFrame);
    problemLabel->setObjectName("contestSectionLabel");

    m_problemListWidget = new QListWidget(contentFrame);
    m_problemListWidget->setObjectName("contestProblemList");
    m_problemListWidget->setProperty("_darkMode", false);
    m_problemListWidget->setMinimumHeight(420);
    m_problemListWidget->setItemDelegate(new ContestProblemDelegate(m_problemListWidget));

    contentLayout->addWidget(problemLabel);
    contentLayout->addWidget(m_problemListWidget, 1);

    bottomLayout->addWidget(m_toolsFrame, 1);
    bottomLayout->addWidget(contentFrame, 4);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    refreshButton->setToolTip("Refresh");
    m_backToolButton->setToolTip("Back");
    m_collapsedBackButton->setToolTip("Back");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(refreshButton, "refresh.svg");
    LightModeIconHelper::applyIcon(m_collapsedBackButton, "back.svg");

    setStyleSheet(
        "ContestPage { background: #f3f1eb; }"
        "#contestTopFrame, #contestLeftFrame, #contestContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#contestTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#contestSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#contestToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#contestToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#contestTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #243029;"
        "}"
        "#contestTopActionButton:hover {"
        "  background: transparent;"
        "}"
        "#contestToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#contestToolButton:hover, #contestToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#contestToolIconButton {"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 36px;"
        "  max-height: 36px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  font-weight: 600;"
        "}"
        "#contestProblemList {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0px;"
        "  padding: 0px;"
        "  outline: none;"
        "}"
        "#contestProblemList::item {"
        "  padding: 12px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#contestProblemList::item:selected {"
        "  background: #dcefea;"
        "  color: #12343b;"
        "}"
        "#contestProblemList::item:hover {"
        "  background: #eef4ef;"
        "}"
    );

    setToolsExpanded(true);

    connect(homeButton, &QPushButton::clicked, this, &ContestPage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(refreshButton, &QPushButton::clicked, this, &ContestPage::refreshRequested);
    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &ContestPage::refreshRequested);
    connect(m_backToolButton, &QPushButton::clicked, this, &ContestPage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &ContestPage::backRequested);
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(
        m_problemListWidget,
        &QListWidget::itemClicked,
        this,
        [this](QListWidgetItem *item) {
            emit problemSelected(
                item->text(),
                item->data(Qt::UserRole).toString());
        });
}

void ContestPage::setToolsExpanded(bool expanded)
{
    m_toolsExpanded = expanded;
    if (m_toolsFrame != nullptr) {
        m_toolsFrame->setMinimumWidth(expanded ? 0 : 84);
        m_toolsFrame->setMaximumWidth(expanded ? QWIDGETSIZE_MAX : 84);
    }
    if (m_toolsPanel != nullptr) {
        m_toolsPanel->setVisible(expanded);
    }
    if (m_collapsedToolsPanel != nullptr) {
        m_collapsedToolsPanel->setVisible(!expanded);
    }
    if (m_collapsedBackButton != nullptr) {
        m_collapsedBackButton->setVisible(!expanded);
    }
    if (m_toolsToggleButton != nullptr) {
        m_toolsToggleButton->setToolTip(expanded ? "Collapse Tools" : "Expand Tools");
        LightModeIconHelper::applyToolsToggleIcon(m_toolsToggleButton, expanded);
    }
}

void ContestPage::openContest(const QString &title, const QString &url)
{
    m_contestTitle = title;
    m_contestUrl = url;
    m_titleLabel->setText(title.isEmpty() ? QString("Contest Set") : title);
    m_problemListWidget->clear();
}

void ContestPage::showLoadingProblems()
{
}

void ContestPage::showContestLoadFailed(const QString &message)
{
    m_problemListWidget->clear();
    auto *item = new QListWidgetItem(message, m_problemListWidget);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
}

void ContestPage::showProblems(const ContestPageInfo &contestPageInfo)
{
    m_problemListWidget->clear();
    m_titleLabel->setText(m_contestTitle.isEmpty() ? QString("Contest Set") : m_contestTitle);
    for (const ContestProblemInfo &problem : contestPageInfo.problems) {
        const QString text = QString("%1 %2").arg(problem.problemId, problem.title);
        auto *item = new QListWidgetItem(text, m_problemListWidget);
        item->setData(Qt::UserRole, problem.problemUrl);
        item->setData(ProblemTextRole, text);
        item->setData(ProblemSolvedRole, problem.solved);
    }
}

void ContestPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    if (m_problemListWidget != nullptr) {
        m_problemListWidget->setProperty("_darkMode", dark);
    }
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "ContestPage { background: #000000; }"
        "#contestTopFrame, #contestLeftFrame, #contestContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#contestTitleLabel, #contestSectionLabel, #contestToolsToggleButton, #contestToolButton, #contestToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#contestTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#contestTopActionButton:hover, #contestToolButton:hover, #contestToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#contestProblemList { color: #e8edf2; }"
        "#contestProblemList::item:selected {"
        "  background: #234257;"
        "  color: #eff8ff;"
        "}"
        "#contestProblemList::item:hover {"
        "  background: #26313c;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
