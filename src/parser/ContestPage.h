#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct ContestProblemInfo
{
    QString problemId;
    QString title;
    QString problemUrl;
    int accept_prople = 0;
    int submission_people = 0;
    bool solved = false;
};

struct ContestPageInfo
{
    QList<ContestProblemInfo> problems;
    int totalProblems = 0;
    int solvedProblems = 0;
};

namespace ContestPageParser
{
ContestPageInfo parseContestPage(const QByteArray &html, const QUrl &baseUrl);
}
