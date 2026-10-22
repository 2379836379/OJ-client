#include "ui/pages/storagepage.h"
#include "ui/lightmodeiconhelper.h"

#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVariantAnimation>
#include <QVBoxLayout>

namespace
{
class SlideSwitch final : public QAbstractButton
{
public:
    explicit SlideSwitch(QWidget *parent = nullptr)
        : QAbstractButton(parent)
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFixedSize(54, 28);

        m_animation = new QVariantAnimation(this);
        m_animation->setDuration(140);
        m_animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            m_offset = value.toReal();
            update();
        });

        connect(this, &QAbstractButton::toggled, this, [this](bool checked) {
            m_animation->stop();
            m_animation->setStartValue(m_offset);
            m_animation->setEndValue(checked ? 1.0 : 0.0);
            m_animation->start();
            update();
        });
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF trackRect(1, 1, width() - 2, height() - 2);
        const QColor trackColor =
            isChecked() ? QColor("#3b82f6") : QColor("#b9c1c9");
        painter.setPen(Qt::NoPen);
        painter.setBrush(trackColor);
        painter.drawRoundedRect(trackRect, trackRect.height() / 2, trackRect.height() / 2);

        const qreal knobSize = height() - 6.0;
        const qreal minX = 3.0;
        const qreal maxX = width() - knobSize - 3.0;
        const qreal knobX = minX + (maxX - minX) * m_offset;
        const QRectF knobRect(knobX, 3.0, knobSize, knobSize);

        painter.setBrush(QColor("#ffffff"));
        painter.drawEllipse(knobRect);
    }

private:
    qreal m_offset = 0.0;
    QVariantAnimation *m_animation = nullptr;
};
}

StoragePage::StoragePage(QWidget *parent)
    : QWidget(parent)
{
    constexpr int kRowHeight = 40;
    constexpr int kRowSpacing = 12;

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("storageTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("Set", topFrame);
    titleLabel->setObjectName("storageTitleLabel");

    topLayout->addWidget(titleLabel, 1);
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("storageTopActionButton");
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("storageTopActionButton");
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("storageLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("storageToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);
    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("storageToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);
    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("storageToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("storageContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(24, 22, 24, 22);
    contentLayout->setSpacing(kRowSpacing);

    auto *cacheRowWidget = new QWidget(contentFrame);
    cacheRowWidget->setFixedHeight(kRowHeight);
    auto *cacheRow = new QHBoxLayout(cacheRowWidget);
    cacheRow->setContentsMargins(0, 0, 0, 0);
    cacheRow->setSpacing(12);
    auto *cacheLabel = new QLabel("Cache Size", cacheRowWidget);
    cacheLabel->setObjectName("storageLabel");
    m_cacheSizeValueLabel = new QLabel("--", cacheRowWidget);
    m_cacheSizeValueLabel->setObjectName("storageValueLabel");
    cacheRow->addWidget(cacheLabel);
    cacheRow->addStretch();
    cacheRow->addWidget(m_cacheSizeValueLabel);

    auto *appRowWidget = new QWidget(contentFrame);
    appRowWidget->setFixedHeight(kRowHeight);
    auto *appRow = new QHBoxLayout(appRowWidget);
    appRow->setContentsMargins(0, 0, 0, 0);
    appRow->setSpacing(12);
    auto *appLabel = new QLabel("Application Size", appRowWidget);
    appLabel->setObjectName("storageLabel");
    m_appSizeValueLabel = new QLabel("--", appRowWidget);
    m_appSizeValueLabel->setObjectName("storageValueLabel");
    appRow->addWidget(appLabel);
    appRow->addStretch();
    appRow->addWidget(m_appSizeValueLabel);

    auto *alarmRowWidget = new QWidget(contentFrame);
    alarmRowWidget->setFixedHeight(kRowHeight);
    auto *alarmRow = new QHBoxLayout(alarmRowWidget);
    alarmRow->setContentsMargins(0, 0, 0, 0);
    alarmRow->setSpacing(12);
    auto *alarmLabel = new QLabel("Alarm", alarmRowWidget);
    alarmLabel->setObjectName("storageLabel");
    m_alarmToggleButton = new SlideSwitch(alarmRowWidget);
    m_alarmToggleButton->setCheckable(true);
    alarmRow->addWidget(alarmLabel);
    alarmRow->addStretch();
    alarmRow->addWidget(m_alarmToggleButton);

    auto *alarmTestRowWidget = new QWidget(contentFrame);
    alarmTestRowWidget->setFixedHeight(kRowHeight);
    auto *alarmTestRow = new QHBoxLayout(alarmTestRowWidget);
    alarmTestRow->setContentsMargins(0, 0, 0, 0);
    alarmTestRow->setSpacing(12);
    auto *alarmTestLabel = new QLabel("Alarm Test", alarmTestRowWidget);
    alarmTestLabel->setObjectName("storageLabel");
    m_alarmTestButton = new QPushButton("alarm test", alarmTestRowWidget);
    m_alarmTestButton->setObjectName("storageClearButton");
    alarmTestRow->addWidget(alarmTestLabel);
    alarmTestRow->addStretch();
    alarmTestRow->addWidget(m_alarmTestButton);

    auto *ringRowWidget = new QWidget(contentFrame);
    ringRowWidget->setFixedHeight(kRowHeight);
    auto *ringRow = new QHBoxLayout(ringRowWidget);
    ringRow->setContentsMargins(0, 0, 0, 0);
    ringRow->setSpacing(12);
    auto *ringLabel = new QLabel("Ring", ringRowWidget);
    ringLabel->setObjectName("storageLabel");
    m_ringPathValueLabel = new QLabel("No file selected", ringRowWidget);
    m_ringPathValueLabel->setObjectName("storagePathLabel");
    m_ringPathValueLabel->setWordWrap(false);
    m_ringPathValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_ringPathButton = new QPushButton("choose", ringRowWidget);
    m_ringPathButton->setObjectName("storageClearButton");
    ringRow->addWidget(ringLabel);
    ringRow->addWidget(m_ringPathValueLabel, 1);
    ringRow->addWidget(m_ringPathButton);

    m_statusLabel = new QLabel(contentFrame);
    m_statusLabel->setObjectName("storageStatusLabel");
    m_statusLabel->setWordWrap(true);

    m_clearCacheButton = new QPushButton("Clear Cache", contentFrame);
    m_clearCacheButton->setObjectName("storageClearButton");

    contentLayout->addWidget(cacheRowWidget);
    contentLayout->addWidget(appRowWidget);
    contentLayout->addWidget(alarmRowWidget);
    contentLayout->addWidget(alarmTestRowWidget);
    contentLayout->addWidget(ringRowWidget);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addStretch();
    contentLayout->addWidget(m_clearCacheButton, 0, Qt::AlignLeft);

    bottomLayout->addWidget(m_toolsFrame, 1);
    bottomLayout->addWidget(contentFrame, 4);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    m_backToolButton->setToolTip("Back");
    m_collapsedBackButton->setToolTip("Back");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(m_collapsedBackButton, "back.svg");

    setStyleSheet(
        "StoragePage { background: #f3f1eb; }"
        "#storageTopFrame, #storageLeftFrame, #storageContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#storageToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#storageToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#storageToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#storageToolButton:hover, #storageToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#storageTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#storageTopActionButton:hover {"
        "  background: transparent;"
        "}"
        "#storageClearButton {"
        "  padding: 10px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#storageClearButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#storageToolIconButton {"
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
        "#storageTitleLabel {"
        "  font-size: 26px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#storageLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#storageValueLabel {"
        "  font-size: 16px;"
        "  color: #1f2328;"
        "}"
        "#storagePathLabel {"
        "  font-size: 14px;"
        "  color: #52606d;"
        "}"
        "#storageStatusLabel {"
        "  min-height: 20px;"
        "  color: #7a4b36;"
        "}"
    );

    setToolsExpanded(true);

    connect(homeButton, &QPushButton::clicked, this, &StoragePage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_backToolButton, &QPushButton::clicked, this, &StoragePage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &StoragePage::backRequested);
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(m_clearCacheButton, &QPushButton::clicked,
            this, &StoragePage::clearCacheRequested);
    connect(m_alarmTestButton, &QPushButton::clicked,
            this, &StoragePage::alarmTestRequested);
    connect(m_ringPathButton, &QPushButton::clicked,
            this, &StoragePage::ringPathPickRequested);
    connect(m_alarmToggleButton, &QAbstractButton::toggled, this, [this](bool checked) {
        m_alarmEnabled = checked;
        if (m_alarmToggleButton != nullptr) {
            m_alarmToggleButton->setChecked(m_alarmEnabled);
        }
        emit alarmToggled(m_alarmEnabled);
    });
}

void StoragePage::showSizes(const QString &cacheSize, const QString &appSize)
{
    m_cacheSizeValueLabel->setText(cacheSize);
    m_appSizeValueLabel->setText(appSize);
    m_clearCacheButton->setEnabled(true);
    m_statusLabel->clear();
}

void StoragePage::showClearSucceeded(const QString &cacheSize, const QString &appSize)
{
    showSizes(cacheSize, appSize);
    m_statusLabel->setText("Cache cleared.");
}

void StoragePage::showOperationFailed(const QString &message)
{
    m_clearCacheButton->setEnabled(true);
    m_statusLabel->setText(message);
}

void StoragePage::showClearing(bool clearing)
{
    m_clearCacheButton->setEnabled(!clearing);
    if (clearing) {
        m_statusLabel->setText("Clearing cache...");
    }
}

void StoragePage::setAlarmEnabled(bool enabled)
{
    m_alarmEnabled = enabled;
    if (m_alarmToggleButton == nullptr) {
        return;
    }

    const QSignalBlocker blocker(m_alarmToggleButton);
    m_alarmToggleButton->setChecked(enabled);
}

void StoragePage::setRingPath(const QString &path)
{
    if (m_ringPathValueLabel == nullptr) {
        return;
    }
    m_ringPathValueLabel->setText(path.trimmed().isEmpty() ? "No file selected" : path);
}

void StoragePage::setToolsExpanded(bool expanded)
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

void StoragePage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "StoragePage { background: #000000; }"
        "#storageTopFrame, #storageLeftFrame, #storageContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#storageTitleLabel, #storageLabel, #storageValueLabel, #storageToolsToggleButton, #storageToolButton, #storageToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#storagePathLabel { color: #a9b7c5; }"
        "#storageTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#storageClearButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#storageTopActionButton:hover, #storageClearButton:hover, #storageToolButton:hover, #storageToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#storageStatusLabel { color: #f0b48a; }";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
