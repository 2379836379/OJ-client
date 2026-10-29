#include "ui/pageshell.h"

#include "ui/uitheme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

PageShell::PageShell(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("pageShell");
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(UiTheme::PageSpacing);

    auto *header = new QFrame(this);
    header->setObjectName("pageShellHeader");
    header->setFixedHeight(UiTheme::HeaderHeight);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(UiTheme::HeaderHorizontalPadding,
                                     UiTheme::HeaderVerticalPadding,
                                     UiTheme::HeaderHorizontalPadding,
                                     UiTheme::HeaderVerticalPadding);
    headerLayout->setSpacing(12);

    m_titleLabel = new QLabel(title, header);
    m_titleLabel->setObjectName("pageShellHeaderTitle");
    headerLayout->addWidget(m_titleLabel, 1);
    m_headerActions = new QHBoxLayout();
    m_headerActions->setContentsMargins(0, 0, 0, 0);
    m_headerActions->setSpacing(8);
    headerLayout->addLayout(m_headerActions);

    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(UiTheme::PageSpacing);

    auto *toolsFrame = new QFrame(this);
    toolsFrame->setObjectName("pageShellTools");
    auto *toolsOuterLayout = new QVBoxLayout(toolsFrame);
    toolsOuterLayout->setContentsMargins(12, 14, 12, 14);
    toolsOuterLayout->setSpacing(10);
    m_toolsToggle = new QPushButton(toolsFrame);
    m_toolsToggle->setObjectName("pageShellToolsToggle");
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
    m_contentLayout->setContentsMargins(UiTheme::PanelHorizontalPadding,
                                       UiTheme::PanelVerticalPadding,
                                       UiTheme::PanelHorizontalPadding,
                                       UiTheme::PanelVerticalPadding);
    m_contentLayout->setSpacing(14);

    bodyLayout->addWidget(toolsFrame, 1);
    bodyLayout->addWidget(contentFrame, 4);
    rootLayout->addWidget(header);
    rootLayout->addLayout(bodyLayout, 1);
    setToolsExpanded(true);
    setDarkMode(false);
}

QHBoxLayout *PageShell::headerActions() const
{
    return m_headerActions;
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

QLabel *PageShell::titleLabel() const
{
    return m_titleLabel;
}

QPushButton *PageShell::addHeaderAction(const QString &text,
                                        const QString &objectName,
                                        const QString &toolTip)
{
    auto *button = new QPushButton(text, this);
    button->setObjectName(objectName.isEmpty()
                              ? QStringLiteral("pageShellActionButton")
                              : objectName);
    if (!toolTip.isEmpty()) {
        button->setToolTip(toolTip);
    }
    m_headerActions->addWidget(button);
    return button;
}

void PageShell::setDarkMode(bool dark)
{
    setStyleSheet(UiTheme::pageShellStyleSheet(dark));
}

void PageShell::setContentFramed(bool framed)
{
    QWidget *contentFrame = m_contentLayout->parentWidget();
    contentFrame->setProperty("framed", framed);
    m_contentLayout->setContentsMargins(framed ? UiTheme::PanelHorizontalPadding : 0,
                                        framed ? UiTheme::PanelVerticalPadding : 0,
                                        framed ? UiTheme::PanelHorizontalPadding : 0,
                                        framed ? UiTheme::PanelVerticalPadding : 0);
    contentFrame->style()->unpolish(contentFrame);
    contentFrame->style()->polish(contentFrame);
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

bool PageShell::toolsExpanded() const
{
    return m_toolsExpanded;
}
