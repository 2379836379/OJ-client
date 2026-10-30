#include "ui/pageshell.h"

#include "ui/lightmodeiconhelper.h"
#include "ui/uitheme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

PageShell::PageShell(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageShell");
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(UiTheme::PageSpacing);

    auto *toolsFrame = new QFrame(this);
    toolsFrame->setObjectName("pageShellTools");
    auto *toolsOuterLayout = new QVBoxLayout(toolsFrame);
    toolsOuterLayout->setContentsMargins(12, 14, 12, 14);
    toolsOuterLayout->setSpacing(8);
    m_toolsToggle = new QPushButton(toolsFrame);
    m_toolsToggle->setObjectName("pageShellToolButton");
    m_toolsToggle->setToolTip("Collapse tools");
    toolsOuterLayout->addWidget(m_toolsToggle);

    m_toolsExpandedPanel = new QWidget(toolsFrame);
    m_toolsLayout = new QVBoxLayout(m_toolsExpandedPanel);
    m_toolsLayout->setContentsMargins(0, 0, 0, 0);
    m_toolsLayout->setSpacing(8);
    toolsOuterLayout->addWidget(m_toolsExpandedPanel, 1);

    m_toolsCollapsedPanel = new QWidget(toolsFrame);
    m_collapsedToolsLayout = new QVBoxLayout(m_toolsCollapsedPanel);
    m_collapsedToolsLayout->setContentsMargins(0, 0, 0, 0);
    m_collapsedToolsLayout->setSpacing(8);
    toolsOuterLayout->addWidget(m_toolsCollapsedPanel, 1);
    connect(m_toolsToggle, &QPushButton::clicked, this, [this]() {
        setToolsExpanded(!m_toolsExpanded);
    });

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("pageShellContent");
    m_contentLayout = new QVBoxLayout(contentFrame);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(14);

    bodyLayout->addWidget(toolsFrame, 1);
    bodyLayout->addWidget(contentFrame, 4);
    rootLayout->addLayout(bodyLayout, 1);
    setToolsExpanded(true);
    setDarkMode(false);
}

QVBoxLayout *PageShell::toolsLayout() const
{
    return m_toolsLayout;
}

QVBoxLayout *PageShell::collapsedToolsLayout() const
{
    return m_collapsedToolsLayout;
}

QVBoxLayout *PageShell::contentLayout() const
{
    return m_contentLayout;
}

QPushButton *PageShell::addTool(const QString &text,
                                const QString &toolTip,
                                const std::function<void()> &action,
                                const QString &iconFile,
                                QPushButton **collapsedButton)
{
    auto *expandedButton = new QPushButton(text, m_toolsExpandedPanel);
    expandedButton->setObjectName("pageShellToolButton");
    expandedButton->setToolTip(toolTip);
    m_toolsLayout->addWidget(expandedButton);

    auto *collapsedToolButton = new QPushButton(m_toolsCollapsedPanel);
    collapsedToolButton->setObjectName("pageShellToolButton");
    collapsedToolButton->setFixedSize(36, 36);
    collapsedToolButton->setToolTip(toolTip);
    LightModeIconHelper::applyIcon(collapsedToolButton, iconFile);
    m_collapsedToolsLayout->addWidget(collapsedToolButton, 0, Qt::AlignHCenter);
    if (collapsedButton != nullptr) {
        *collapsedButton = collapsedToolButton;
    }

    connect(expandedButton, &QPushButton::clicked, this, [action]() {
        if (action) {
            action();
        }
    });
    connect(collapsedToolButton, &QPushButton::clicked, this, [action]() {
        if (action) {
            action();
        }
    });
    return expandedButton;
}

void PageShell::setDarkMode(bool dark)
{
    setStyleSheet(UiTheme::pageShellStyleSheet(dark));
}

void PageShell::setToolsExpanded(bool expanded)
{
    m_toolsExpanded = expanded;
    m_toolsExpandedPanel->setVisible(expanded);
    m_toolsCollapsedPanel->setVisible(!expanded);
    const int width = expanded ? UiTheme::ToolRailExpandedWidth
                               : UiTheme::ToolRailCollapsedWidth;
    QWidget *toolsFrame = m_toolsExpandedPanel->parentWidget();
    toolsFrame->setMinimumWidth(width);
    toolsFrame->setMaximumWidth(width);
    m_toolsToggle->setText(expanded ? QStringLiteral("Collapse")
                                    : QStringLiteral("Expand"));
    m_toolsToggle->setToolTip(expanded ? QStringLiteral("Collapse tools")
                                      : QStringLiteral("Expand tools"));
}
