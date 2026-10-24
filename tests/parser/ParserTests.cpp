#include "parser/ContestPage.h"
#include "parser/GroupHomepage.h"
#include "parser/GroupPage.h"
#include "parser/Homepage.h"
#include "parser/ProblemPage.h"
#include "parser/ResultPage.h"
#include "parser/SubmitPage.h"
#include "parser/UserHomepage.h"

#include <QCoreApplication>
#include <QFile>
#include <QStringConverter>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
QByteArray loadFixture(const QString &name)
{
    QFile file(QStringLiteral(OJ_HTML_EXAMPLES_DIR) + QLatin1Char('/') + name);
    if (!file.open(QIODevice::ReadOnly)) {
        QTextStream(stderr) << "Failed to load fixture: " << file.fileName()
                            << Qt::endl;
        return QByteArray();
    }
    return file.readAll();
}

void field(QTextStream &out, const QString &name, const QString &value)
{
    out << name << " = " << value << Qt::endl;
}

void field(QTextStream &out, const QString &name, int value)
{
    out << name << " = " << value << Qt::endl;
}

void field(QTextStream &out, const QString &name, qsizetype value)
{
    out << name << " = " << value << Qt::endl;
}

void field(QTextStream &out, const QString &name, bool value)
{
    out << name << " = " << (value ? QStringLiteral("true")
                                   : QStringLiteral("false"))
        << Qt::endl;
}

void section(QTextStream &out, const QString &name)
{
    out << "\n========== " << name << " ==========\n";
}

void printHomepage(QTextStream &out)
{
    section(out, QStringLiteral("Homepage"));
    const QByteArray html = loadFixture(QStringLiteral("Homepage.html"));
    field(out, QStringLiteral("personalHomeUrl"),
          HomepageParser::parseHomepage(
              html, QUrl(QStringLiteral("http://openjudge.cn/"))));
}

void printGroupHomepage(QTextStream &out)
{
    section(out, QStringLiteral("GroupHomepage"));
    const QByteArray html = loadFixture(QStringLiteral("GroupHomepage.html"));
    field(out, QStringLiteral("groupUrl"),
          GroupHomepageParser::parseGroupHomepage(html));
}

void printUserHomepage(QTextStream &out)
{
    section(out, QStringLiteral("UserHomepage"));
    const QByteArray html = loadFixture(QStringLiteral("UserHomepage.html"));
    const QList<JoinedClassInfo> groups =
        UserHomepageParser::parseUserHomepage(html);
    field(out, QStringLiteral("groups.count"), groups.size());
    for (qsizetype i = 0; i < groups.size(); ++i) {
        const QString prefix = QStringLiteral("groups[%1].").arg(i);
        field(out, prefix + QStringLiteral("name"), groups.at(i).name);
        field(out, prefix + QStringLiteral("url"), groups.at(i).url);
    }
}

void printGroupPage(QTextStream &out)
{
    section(out, QStringLiteral("GroupPage"));
    const QByteArray html = loadFixture(QStringLiteral("GroupPage.html"));
    const GroupPageInfo page = GroupPageParser::parseGroupPage(
        html, QUrl(QStringLiteral("http://dxfjgc.openjudge.cn/")));
    field(out, QStringLiteral("groupPageUrl"), page.groupPageUrl);
    field(out, QStringLiteral("contestSets.count"), page.contestSets.size());
    for (qsizetype i = 0; i < page.contestSets.size(); ++i) {
        const ContestSetInfo &item = page.contestSets.at(i);
        const QString prefix = QStringLiteral("contestSets[%1].").arg(i);
        field(out, prefix + QStringLiteral("title"), item.title);
        field(out, prefix + QStringLiteral("url"), item.url);
        field(out, prefix + QStringLiteral("itemClass"), item.itemClass);
        field(out, prefix + QStringLiteral("problemnumber"), item.problemnumber);
        field(out, prefix + QStringLiteral("endtime"), item.endtime);
        field(out, prefix + QStringLiteral("extraText"), item.extraText);
    }
}

void printContestPage(QTextStream &out)
{
    section(out, QStringLiteral("ContestPage"));
    const QByteArray html = loadFixture(QStringLiteral("ContestPage.html"));
    const ContestPageInfo page = ContestPageParser::parseContestPage(
        html, QUrl(QStringLiteral("http://dxfjgc.openjudge.cn/")));
    field(out, QStringLiteral("contestPageUrl"), page.contestPageUrl);
    field(out, QStringLiteral("totalProblems"), page.totalProblems);
    field(out, QStringLiteral("solvedProblems"), page.solvedProblems);
    for (qsizetype i = 0; i < page.problems.size(); ++i) {
        const ContestProblemInfo &problem = page.problems.at(i);
        const QString prefix = QStringLiteral("problems[%1].").arg(i);
        field(out, prefix + QStringLiteral("problemId"), problem.problemId);
        field(out, prefix + QStringLiteral("title"), problem.title);
        field(out, prefix + QStringLiteral("problemUrl"), problem.problemUrl);
        field(out, prefix + QStringLiteral("accept_prople"),
              problem.accept_prople);
        field(out, prefix + QStringLiteral("submission_people"),
              problem.submission_people);
        field(out, prefix + QStringLiteral("solved"), problem.solved);
    }
}

void printProblemPage(QTextStream &out)
{
    section(out, QStringLiteral("ProblemPage"));
    const QByteArray html = loadFixture(QStringLiteral("ProblemPage.html"));
    const ProblemPageInfo page = ProblemPageParser::parseProblemPage(
        html, QUrl(QStringLiteral("http://dxfjgc.openjudge.cn/2026autumn05/1/")));
    field(out, QStringLiteral("problemUrl"), page.problemUrl);
    field(out, QStringLiteral("title"), page.title);
    field(out, QStringLiteral("submitUrl"), page.submitUrl);
    field(out, QStringLiteral("timeLimit"), page.timeLimit);
    field(out, QStringLiteral("memoryLimit"), page.memoryLimit);
    field(out, QStringLiteral("description"), page.description);
    field(out, QStringLiteral("starterCode"), page.starterCode);
    field(out, QStringLiteral("inputSpec"), page.inputSpec);
    field(out, QStringLiteral("outputSpec"), page.outputSpec);
    field(out, QStringLiteral("sampleInput"), page.sampleInput);
    field(out, QStringLiteral("sampleOutput"), page.sampleOutput);
    field(out, QStringLiteral("hint"), page.hint);
    field(out, QStringLiteral("tried_people"), page.tried_people);
    field(out, QStringLiteral("passed_people"), page.passed_people);
}

void printResultPage(QTextStream &out)
{
    section(out, QStringLiteral("ResultPage"));
    const QByteArray html = loadFixture(QStringLiteral("ResultPage.html"));
    const ResultPageInfo page = ResultPageParser::parseResultPage(
        html, QUrl(QStringLiteral("http://dxfjgc.openjudge.cn/")));
    field(out, QStringLiteral("pageUrl"), page.pageUrl);
    field(out, QStringLiteral("solutionUrl"), page.solutionUrl);
    field(out, QStringLiteral("submissionId"), page.submissionId);
    field(out, QStringLiteral("statusText"), page.statusText);
    field(out, QStringLiteral("statusClass"), page.statusClass);
    field(out, QStringLiteral("detailTitle"), page.detailTitle);
    field(out, QStringLiteral("detailText"), page.detailText);
    field(out, QStringLiteral("hasResult"), page.hasResult);
}

void printSubmitPage(QTextStream &out)
{
    section(out, QStringLiteral("SubmitPage"));
    const QByteArray html = loadFixture(QStringLiteral("SubmitPage.html"));
    const SubmitPageInfo page = SubmitPageParser::parseSubmitPage(
        html, QUrl(QStringLiteral("http://dxfjgc.openjudge.cn/2026autumn05/1/submit/")));
    field(out, QStringLiteral("pageUrl"), page.pageUrl);
    field(out, QStringLiteral("submitActionUrl"), page.submitActionUrl);
    field(out, QStringLiteral("contestId"), page.contestId);
    field(out, QStringLiteral("problemNumber"), page.problemNumber);
    field(out, QStringLiteral("sourceEncode"), page.sourceEncode);
    field(out, QStringLiteral("languages.count"), page.languages.size());
    for (qsizetype i = 0; i < page.languages.size(); ++i) {
        const SubmitLanguageOption &language = page.languages.at(i);
        const QString prefix = QStringLiteral("languages[%1].").arg(i);
        field(out, prefix + QStringLiteral("value"), language.value);
        field(out, prefix + QStringLiteral("label"), language.label);
        field(out, prefix + QStringLiteral("checked"), language.checked);
    }
    field(out, QStringLiteral("defaultLanguage"),
          SubmitPageParser::defaultLanguage(page));
}
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);

#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif

    printHomepage(out);
    printGroupHomepage(out);
    printUserHomepage(out);
    printGroupPage(out);
    printContestPage(out);
    printProblemPage(out);
    printResultPage(out);
    printSubmitPage(out);
    return 0;
}
