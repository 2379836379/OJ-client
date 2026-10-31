#include "ui/pages/classpage.h"

#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

namespace
{
constexpr int ContestEndTimeRole = Qt::UserRole + 1;

class ContestSetDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.text.clear();

        const QWidget *widget = itemOption.widget;
        const QStyle *style = widget != nullptr ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, widget);

        const QString endTime = index.data(ContestEndTimeRole).toString();
        const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText,
                                                     &itemOption,
                                                     widget);
        painter->save();
        painter->setPen(itemOption.palette.color(QPalette::Text));
        if (endTime.isEmpty()) {
            painter->drawText(textRect,
                              Qt::AlignVCenter | Qt::AlignLeft,
                              index.data(Qt::DisplayRole).toString());
        } else {
            const int dateWidth = qMin(textRect.width() / 2, 220);
            const QRect titleRect = textRect.adjusted(0, 0, -dateWidth - 16, 0);
            const QRect dateRect(textRect.right() - dateWidth + 1,
                                 textRect.top(),
                                 dateWidth,
                                 textRect.height());
            painter->drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft,
                              index.data(Qt::DisplayRole).toString());
            painter->drawText(dateRect, Qt::AlignVCenter | Qt::AlignRight, endTime);
        }
        painter->restore();
    }
};

QString contestListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 0px; border-radius: 4px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: transparent; color: %1; }"
               "QListWidget::item:hover { background: %2; }")
        .arg(colors.text, colors.hover);
}
}

ClassPage::ClassPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    m_shell->contentLayout()->setContentsMargins(20,18,20,18);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);

    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    m_titleLabel = new QLabel("Course", m_shell);
    m_titleLabel->setObjectName("classTitleLabel");
    m_contestListWidget = new QListWidget(m_shell);
    m_contestListWidget->setItemDelegate(new ContestSetDelegate(m_contestListWidget));
    m_contestListWidget->setMinimumHeight(420);

    m_shell->contentLayout()->addWidget(m_titleLabel);
    m_shell->contentLayout()->addWidget(m_contestListWidget, 1);

    connect(m_contestListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit contestSelected(item->text(), item->data(Qt::UserRole).toString());
    });
    setDarkMode(false);
}

void ClassPage::openClass(const QString &name, const QString &url)
{
    m_className = name;
    m_classUrl = url;
    m_titleLabel->setText(name.isEmpty() ? QStringLiteral("Course") : name);
    m_contestListWidget->clear();
}

void ClassPage::showLoadingContests()
{
}

void ClassPage::showClassLoadFailed(const QString &message)
{
    m_contestListWidget->clear();
    auto *item = new QListWidgetItem(message, m_contestListWidget);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
}

void ClassPage::showContestSets(const GroupPageInfo &groupPageInfo)
{
    m_contestListWidget->clear();
    for (const ContestSetInfo &contestSet : groupPageInfo.contestSets) {
        auto *item = new QListWidgetItem(contestSet.title, m_contestListWidget);
        item->setData(Qt::UserRole, contestSet.url);
        item->setData(ContestEndTimeRole, contestSet.endtime);
    }
}

void ClassPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    const UiTheme::Palette colors = UiTheme::palette(dark);
    m_shell->setStyleSheet(
        UiTheme::pageShellStyleSheet(dark)
        + QString("#classTitleLabel { color: %1; font-size: 24px; font-weight: 600; }"
                  "#classSectionLabel { color: %1; font-size: 16px; font-weight: 600; }")
              .arg(colors.text));
    m_contestListWidget->setStyleSheet(contestListStyle(dark));
}
