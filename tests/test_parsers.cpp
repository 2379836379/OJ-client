#include "parser/contestparser.h"
#include "parser/parsercommon.h"
#include "parser/problemparser.h"

#include <QObject>
#include <QTest>

class TestParsers : public QObject
{
    Q_OBJECT

private slots:
    void parserCommonDecodesUtf8();
    void problemParserExtractsCoreFields();
    void problemParserHandlesEmptyInput();
    void contestParserHandlesEmptyInput();
};

void TestParsers::parserCommonDecodesUtf8()
{
    const QString original = QStringLiteral("测试 UTF-8 内容");
    const QByteArray utf8 = original.toUtf8();
    QCOMPARE(ParserCommon::toUtf8String(utf8), original);
}

void TestParsers::problemParserExtractsCoreFields()
{
    const QByteArray html =
        "<html><body>"
        "<div id=\"pageTitle\"><h2>A+B Problem</h2></div>"
        "<a href=\"/practice/1000/submit/\">提交</a>"
        "<dl>"
        "<dt>总时间限制:</dt><dd>1000ms</dd>"
        "<dt>内存限制:</dt><dd>65536kB</dd>"
        "<dt>描述</dt><dd>计算两个整数之和<pre>int a, b;</pre></dd>"
        "<dt>输入</dt><dd>两个整数 a 和 b</dd>"
        "<dt>输出</dt><dd>a 与 b 的和</dd>"
        "<dt>样例输入</dt><dd><pre>1 2</pre></dd>"
        "<dt>样例输出</dt><dd><pre>3</pre></dd>"
        "<dt>提示</dt><dd>注意整数溢出</dd>"
        "<dt>尝试人数</dt><dd>100</dd>"
        "<dt>通过人数</dt><dd>80</dd>"
        "</dl>"
        "</body></html>";

    const ProblemPageInfo info = ProblemParser::parseProblemPage(
        html, QUrl("http://openjudge.cn/practice/1000/"));

    QCOMPARE(info.title, QStringLiteral("A+B Problem"));
    QVERIFY(info.submitUrl.endsWith("/practice/1000/submit/"));
    QCOMPARE(info.timeLimit, QStringLiteral("1000ms"));
    QCOMPARE(info.memoryLimit, QStringLiteral("65536kB"));
    QCOMPARE(info.sampleInput, QStringLiteral("1 2"));
    QCOMPARE(info.sampleOutput, QStringLiteral("3"));
    QCOMPARE(info.tried_people, 100);
    QCOMPARE(info.passed_people, 80);
    QVERIFY(info.starterCode.contains("int a, b;"));
    QVERIFY(info.inputSpec.contains(QStringLiteral("两个整数")));
}

void TestParsers::problemParserHandlesEmptyInput()
{
    const ProblemPageInfo info =
        ProblemParser::parseProblemPage(QByteArray(), QUrl("http://openjudge.cn/x/"));
    QCOMPARE(info.problemUrl, QStringLiteral("http://openjudge.cn/x/"));
    QVERIFY(info.title.isEmpty());
    QCOMPARE(info.tried_people, 0);
    QCOMPARE(info.passed_people, 0);
}

void TestParsers::contestParserHandlesEmptyInput()
{
    const ContestPageInfo info =
        ContestParser::parseContestPage(QByteArray(), QUrl("http://openjudge.cn/c/"));
    QCOMPARE(info.totalProblems, 0);
    QCOMPARE(info.solvedProblems, 0);
    QVERIFY(info.problems.isEmpty());
}

QTEST_APPLESS_MAIN(TestParsers)

#include "test_parsers.moc"
