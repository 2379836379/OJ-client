#include "ui/pages/reviewpage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
constexpr int ProblemUrlRole = Qt::UserRole;
constexpr int ProblemTitleRole = Qt::UserRole + 1;

struct GradeInfo
{
    QString label;
    int grade;
};

const QList<GradeInfo> &gradeInfos()
{
    static const QList<GradeInfo> infos = {
        {"重来", 0},
        {"困难", 1},
        {"良好", 2},
        {"简单", 3}};
    return infos;
}
}

ReviewPage::ReviewPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);
    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *contentLayout = m_shell->contentLayout();
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(12);
    auto *titleLabel = new QLabel("Review", m_shell);
    titleLabel->setObjectName("reviewTitleLabel");
    contentLayout->addWidget(titleLabel);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    auto *listFrame = new QFrame(m_shell);
    listFrame->setObjectName("reviewListFrame");
    auto *listOuter = new QVBoxLayout(listFrame);
    listOuter->setContentsMargins(20, 18, 20, 18);
    listOuter->setSpacing(12);
    m_statusLabel = new QLabel("今日待复习", listFrame);
    m_statusLabel->setObjectName("reviewSectionLabel");
    m_dueList = new QListWidget(listFrame);
    m_dueList->setObjectName("reviewDueList");
    m_dueList->setMinimumHeight(360);
    listOuter->addWidget(m_statusLabel);
    listOuter->addWidget(m_dueList, 1);

    auto *detailFrame = new QFrame(m_shell);
    detailFrame->setObjectName("reviewDetailFrame");
    auto *detailOuter = new QVBoxLayout(detailFrame);
    detailOuter->setContentsMargins(20, 18, 20, 18);
    detailOuter->setSpacing(12);
    m_detailTitle = new QLabel("选择左侧题目开始复习", detailFrame);
    m_detailTitle->setObjectName("reviewSectionLabel");
    m_detailTitle->setWordWrap(true);
    m_detailMeta = new QLabel(detailFrame);
    m_detailMeta->setObjectName("reviewDetailMeta");
    m_detailMeta->setWordWrap(true);
    m_detailNote = new QPlainTextEdit(detailFrame);
    m_detailNote->setObjectName("reviewDetailNote");
    m_detailNote->setReadOnly(true);
    m_detailNote->setPlaceholderText("这道题没有笔记。");

    auto *gradeLayout = new QHBoxLayout();
    gradeLayout->setSpacing(10);
    for (const GradeInfo &info : gradeInfos()) {
        auto *button = new QPushButton(info.label, detailFrame);
        button->setObjectName("reviewGradeButton");
        const int grade = info.grade;
        connect(button, &QPushButton::clicked, this, [this, grade]() {
            if (!m_currentUrl.isEmpty()) {
                emit gradeRequested(m_currentUrl, grade);
            }
        });
        m_gradeButtons << button;
        gradeLayout->addWidget(button);
    }

    m_openButton = new QPushButton("打开题目", detailFrame);
    m_openButton->setObjectName("reviewOpenButton");
    connect(m_openButton, &QPushButton::clicked, this, [this]() {
        if (!m_currentUrl.isEmpty()) {
            emit problemSelected(m_currentTitle, m_currentUrl);
        }
    });

    detailOuter->addWidget(m_detailTitle);
    detailOuter->addWidget(m_detailMeta);
    detailOuter->addWidget(m_detailNote, 1);
    detailOuter->addLayout(gradeLayout);
    detailOuter->addWidget(m_openButton, 0, Qt::AlignLeft);

    bottomLayout->addWidget(listFrame, 1);
    bottomLayout->addWidget(detailFrame, 1);
    contentLayout->addLayout(bottomLayout, 1);
    m_shell->setStyleSheet(UiTheme::pageShellStyleSheet(false)
                           + "#pageShellContent { background: transparent; border: none; }");

    setStyleSheet(
        "#reviewListFrame, #reviewDetailFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#reviewTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#reviewSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#reviewDetailMeta { color: #5e675f; }"
        "#reviewDueList {"
        "  background: transparent;"
        "  border: none;"
        "  outline: none;"
        "  color: #1f2328;"
        "}"
        "#reviewDueList::item {"
        "  padding: 10px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#reviewDueList::item:selected { background: #dcefea; color: #12343b; }"
        "#reviewDueList::item:hover { background: #eef4ef; }"
        "#reviewDetailNote {"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 10px;"
        "  background: #ffffff;"
        "  color: #1f2328;"
        "  padding: 8px;"
        "}"
        "#reviewGradeButton, #reviewOpenButton {"
        "  padding: 8px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#reviewGradeButton:hover, #reviewOpenButton:hover { background: #eef4ef; }"
        "#reviewGradeButton:disabled { color: #b7b1a4; }");

    connect(m_dueList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item == nullptr) {
            return;
        }
        const int row = m_dueList->row(item);
        if (row >= 0 && row < m_dueProblems.size()) {
            showDetail(m_dueProblems.at(row));
        }
    });

    clearDetail();
}

void ReviewPage::showDue(const QList<ProblemMeta> &dueProblems)
{
    m_dueProblems = dueProblems;
    m_dueList->clear();
    for (const ProblemMeta &meta : m_dueProblems) {
        const QString displayText = meta.title.isEmpty() ? meta.problemUrl : meta.title;
        auto *item = new QListWidgetItem(displayText, m_dueList);
        item->setData(ProblemUrlRole, meta.problemUrl);
        item->setData(ProblemTitleRole, meta.title);
        if (!meta.tags.isEmpty()) {
            item->setToolTip(meta.tags.join(", "));
        }
    }

    if (m_dueProblems.isEmpty()) {
        m_statusLabel->setText("今日没有待复习题目。在题目页勾选「加入错题本」即可排程。");
    } else {
        m_statusLabel->setText(QString("今日待复习 %1 题").arg(m_dueProblems.size()));
    }
    clearDetail();
}

void ReviewPage::showDetail(const ProblemMeta &meta)
{
    m_currentUrl = meta.problemUrl;
    m_currentTitle = meta.title;
    m_detailTitle->setText(meta.title.isEmpty() ? meta.problemUrl : meta.title);

    const QString difficultyText =
        meta.difficulty > 0 ? QString::number(meta.difficulty) : QStringLiteral("未设置");
    const QString tagsText = meta.tags.isEmpty() ? QStringLiteral("无") : meta.tags.join(", ");
    m_detailMeta->setText(QString("难度: %1 | 标签: %2 | 已复习: %3 次")
                              .arg(difficultyText, tagsText, QString::number(meta.reviewCount)));
    m_detailNote->setPlainText(meta.note);
    setGradeButtonsEnabled(true);
    m_openButton->setEnabled(true);
}

void ReviewPage::clearDetail()
{
    m_currentUrl.clear();
    m_currentTitle.clear();
    m_detailTitle->setText("选择左侧题目开始复习");
    m_detailMeta->clear();
    m_detailNote->clear();
    setGradeButtonsEnabled(false);
    if (m_openButton != nullptr) {
        m_openButton->setEnabled(false);
    }
}

void ReviewPage::setGradeButtonsEnabled(bool enabled)
{
    for (QPushButton *button : m_gradeButtons) {
        button->setEnabled(enabled);
    }
}

void ReviewPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setStyleSheet(UiTheme::pageShellStyleSheet(dark)
                           + "#pageShellContent { background: transparent; border: none; }");
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "#reviewListFrame, #reviewDetailFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#reviewTitleLabel, #reviewSectionLabel { color: #d9e1e8; }"
        "#reviewDetailMeta { color: #9ba8b6; }"
        "#reviewDueList { color: #e8edf2; }"
        "#reviewDueList::item:selected { background: #234257; color: #eff8ff; }"
        "#reviewDueList::item:hover { background: #26313c; }"
        "#reviewDetailNote {"
        "  border: 1px solid #2c3844;"
        "  background: #11171d;"
        "  color: #e8edf2;"
        "}"
        "#reviewGradeButton, #reviewOpenButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#reviewGradeButton:hover, #reviewOpenButton:hover { background: #26313c; }"
        "#reviewGradeButton:disabled { color: #5a6675; }";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
