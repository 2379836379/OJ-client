#pragma once

#include <QWidget>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;

class PageShell : public QWidget
{
public:
    explicit PageShell(const QString &title, QWidget *parent = nullptr);

    QHBoxLayout *headerActions() const;
    QVBoxLayout *toolsLayout() const;
    QVBoxLayout *collapsedToolsLayout() const;
    QVBoxLayout *contentLayout() const;

    QLabel *titleLabel() const;
    QPushButton *addHeaderAction(const QString &text,
                                 const QString &objectName = QString(),
                                 const QString &toolTip = QString());
    void setDarkMode(bool dark);
    void setContentFramed(bool framed);
    void setToolsExpanded(bool expanded);
    bool toolsExpanded() const;

private:
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_toolsToggle = nullptr;
    QWidget *m_toolsExpandedPanel = nullptr;
    QWidget *m_toolsCollapsedPanel = nullptr;
    QHBoxLayout *m_headerActions = nullptr;
    QVBoxLayout *m_toolsLayout = nullptr;
    QVBoxLayout *m_collapsedToolsLayout = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    bool m_toolsExpanded = true;
};
