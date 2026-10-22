#pragma once

#include "parser/classparser.h"
#include "parser/groupparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
struct NetworkResult;

class ClassRepository : public QObject
{
    Q_OBJECT

public:
    explicit ClassRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchContestSets(const QUrl &classPageUrl);

signals:
    void contestSetsFetched(const ClassPageInfo &classPageInfo,
                            const GroupPageInfo &groupPageInfo);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedClassPageUrl;
    QUrl m_expectedGroupPageUrl;
    ClassPageInfo m_lastClassPageInfo;
};
