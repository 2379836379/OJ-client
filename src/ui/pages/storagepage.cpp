#include "ui/pages/storagepage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

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

    m_shell = new PageShell(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);
    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *contentLayout = m_shell->contentLayout();
    contentLayout->setContentsMargins(24, 22, 24, 22);
    contentLayout->setSpacing(kRowSpacing);
    auto *titleLabel = new QLabel("Set", m_shell);
    titleLabel->setObjectName("storageTitleLabel");
    contentLayout->addWidget(titleLabel);

    auto *cacheRowWidget = new QWidget(m_shell);
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

    auto *appRowWidget = new QWidget(m_shell);
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

    auto *alarmRowWidget = new QWidget(m_shell);
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

    auto *alarmTestRowWidget = new QWidget(m_shell);
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

    auto *ringRowWidget = new QWidget(m_shell);
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

    m_statusLabel = new QLabel(m_shell);
    m_statusLabel->setObjectName("storageStatusLabel");
    m_statusLabel->setWordWrap(true);

    m_clearCacheButton = new QPushButton("Clear Cache", m_shell);
    m_clearCacheButton->setObjectName("storageClearButton");

    contentLayout->addWidget(cacheRowWidget);
    contentLayout->addWidget(appRowWidget);
    contentLayout->addWidget(alarmRowWidget);
    contentLayout->addWidget(alarmTestRowWidget);
    contentLayout->addWidget(ringRowWidget);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addStretch();
    contentLayout->addWidget(m_clearCacheButton, 0, Qt::AlignLeft);

    m_shell->setStyleSheet(UiTheme::pageShellStyleSheet(false));

    setStyleSheet(
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

void StoragePage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "#storageTitleLabel, #storageLabel, #storageValueLabel {"
        "  color: #d9e1e8;"
        "}"
        "#storagePathLabel { color: #a9b7c5; }"
        "#storageClearButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#storageClearButton:hover { background: #26313c; }"
        "#storageStatusLabel { color: #f0b48a; }";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
