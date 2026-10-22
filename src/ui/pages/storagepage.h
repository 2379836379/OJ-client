#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QFrame;
class QWidget;
class QAbstractButton;

class StoragePage : public QWidget
{
    Q_OBJECT

public:
    explicit StoragePage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showSizes(const QString &cacheSize, const QString &appSize);
    void showClearSucceeded(const QString &cacheSize, const QString &appSize);
    void showOperationFailed(const QString &message);
    void showClearing(bool clearing);
    void setAlarmEnabled(bool enabled);
    void setRingPath(const QString &path);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void clearCacheRequested();
    void alarmTestRequested();
    void ringPathPickRequested();
    void alarmToggled(bool enabled);

private:
    void setToolsExpanded(bool expanded);

    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QLabel *m_cacheSizeValueLabel = nullptr;
    QLabel *m_appSizeValueLabel = nullptr;
    QAbstractButton *m_alarmToggleButton = nullptr;
    QLabel *m_ringPathValueLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_alarmTestButton = nullptr;
    QPushButton *m_ringPathButton = nullptr;
    QPushButton *m_clearCacheButton = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
    bool m_alarmEnabled = false;
};
