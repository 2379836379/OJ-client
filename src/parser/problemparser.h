#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

struct ProblemPageInfo
{
    QString problemUrl;
    QString title;
    QString submitUrl;
    QString timeLimit;
    QString memoryLimit;
    QString description;
    QString starterCode;
    QString inputSpec;
    QString outputSpec;
    QString sampleInput;
    QString sampleOutput;
    QString hint;
    int tried_people;
    int passed_people;
};

namespace ProblemParser
{
ProblemPageInfo parseProblemPage(const QByteArray &html, const QUrl &baseUrl);
}
