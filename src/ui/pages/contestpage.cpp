#include "ui/pages/contestpage.h"

#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QApplication>
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
constexpr int ContentPaddingX = 20;
constexpr int ContentPaddingY = 18;

class ContestProblemDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

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
        const QString status = solved ? QStringLiteral("finished") : QString();
        const bool dark = opt.widget != nullptr && opt.widget->property("_darkMode").toBool();
        QColor titleColor = opt.palette.color(QPalette::Text);
        QColor statusColor = solved ? (dark ? QColor("#7CFF8A") : QColor("#23C552"))
                                    : opt.palette.color(QPalette::Mid);
        if (opt.state & QStyle::State_Selected) {
            titleColor = dark ? QColor("#eff8ff") : QColor("#12343b");
            statusColor = solved ? (dark ? QColor("#9BFF8C") : QColor("#2DD85F"))
                                 : titleColor;
        }

        const int statusWidth = status.isEmpty() ? 0 : QFontMetrics(opt.font).horizontalAdvance(status);
        QRect titleRect = contentRect;
        if (statusWidth > 0) {
            titleRect.setRight(contentRect.right() - statusWidth - 16);
        }
        painter->setFont(opt.font);
        painter->setPen(titleColor);
        painter->drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft,
                          QFontMetrics(opt.font).elidedText(title, Qt::ElideRight, titleRect.width()));
        if (!status.isEmpty()) {
            QFont statusFont = opt.font;
            statusFont.setBold(true);
            painter->setFont(statusFont);
            painter->setPen(statusColor);
            painter->drawText(contentRect, Qt::AlignVCenter | Qt::AlignRight, status);
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

QString problemListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 4px; border-radius: 8px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: %2; color: %1; }"
               "QListWidget::item:hover { background: %3; }")
        .arg(colors.text, colors.selected, colors.hover);
}
}

ContestPage::ContestPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    m_shell->contentLayout()->setContentsMargins(ContentPaddingX,
                                                  ContentPaddingY,
                                                  ContentPaddingX,
                                                  ContentPaddingY);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);

    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_themeToolButton = m_shell->addTool("Dark Mode", "Dark Mode", [this]() {
        emit themeToggleRequested(!m_darkMode);
    }, "dark-mode.svg", &m_collapsedThemeToolButton);
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    m_titleLabel = new QLabel("Contest Set", m_shell);
    m_titleLabel->setObjectName("contestTitleLabel");
    auto *problemLabel = new QLabel("Problem List", m_shell);
    problemLabel->setObjectName("contestSectionLabel");
    m_problemListWidget = new QListWidget(m_shell);
    m_problemListWidget->setProperty("_darkMode", false);
    m_problemListWidget->setMinimumHeight(420);
    m_problemListWidget->setItemDelegate(new ContestProblemDelegate(m_problemListWidget));

    m_shell->contentLayout()->addWidget(m_titleLabel);
    m_shell->contentLayout()->addWidget(problemLabel);
    m_shell->contentLayout()->addWidget(m_problemListWidget, 1);

    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &ContestPage::refreshRequested);
    connect(m_problemListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit problemSelected(item->text(), item->data(Qt::UserRole).toString());
    });
    setDarkMode(false);
}

void ContestPage::openContest(const QString &title, const QString &url)
{
    m_contestTitle = title;
    m_contestUrl = url;
    m_titleLabel->setText(title.isEmpty() ? QStringLiteral("Contest Set") : title);
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
    m_titleLabel->setText(m_contestTitle.isEmpty() ? QStringLiteral("Contest Set") : m_contestTitle);
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
    m_problemListWidget->setProperty("_darkMode", dark);
    const QString themeLabel = dark ? QStringLiteral("Light Mode")
                                    : QStringLiteral("Dark Mode");
    m_themeToolButton->setText(themeLabel);
    m_themeToolButton->setToolTip(themeLabel);
    m_collapsedThemeToolButton->setToolTip(themeLabel);

    const UiTheme::Palette colors = UiTheme::palette(dark);
    m_shell->setStyleSheet(
        UiTheme::pageShellStyleSheet(dark)
        + QString("#contestTitleLabel { color: %1; font-size: 24px; font-weight: 600; }"
                  "#contestSectionLabel { color: %1; font-size: 16px; font-weight: 600; }")
              .arg(colors.text));
    m_problemListWidget->setStyleSheet(problemListStyle(dark));
}
