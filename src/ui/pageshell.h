#pragma once

#include <QWidget>

#include <functional>

class QVBoxLayout;
class QPushButton;

class PageShell : public QWidget
{
public:
    explicit PageShell(QWidget *parent = nullptr);

    QVBoxLayout *toolsLayout() const;
    QVBoxLayout *collapsedToolsLayout() const;
    QVBoxLayout *contentLayout() const;
    void addTool(const QString &text,
                 const QString &toolTip,
                 const std::function<void()> &action,
                 const QString &iconFile = QString());

    void setDarkMode(bool dark);

private:
    void setToolsExpanded(bool expanded);

    QPushButton *m_toolsToggle = nullptr;
    QWidget *m_toolsExpandedPanel = nullptr;
    QWidget *m_toolsCollapsedPanel = nullptr;
    QVBoxLayout *m_toolsLayout = nullptr;
    QVBoxLayout *m_collapsedToolsLayout = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    bool m_toolsExpanded = true;
};
