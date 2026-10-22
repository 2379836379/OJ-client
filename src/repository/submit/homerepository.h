#pragma once

#include "parser/loginparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
struct NetworkResult;

class HomeRepository : public QObject
{
    Q_OBJECT

public:
    explicit HomeRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchJoinedClasses(const QUrl &userHomeUrl);

signals:
    void joinedClassesFetched(const QList<JoinedClassInfo> &classes);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedUserHomeUrl;
};
