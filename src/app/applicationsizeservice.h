#pragma once

#include <QObject>
#include <QString>
#include <QtGlobal>

class ApplicationSizeService : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationSizeService(QObject *parent = nullptr);

    qint64 totalApplicationSizeBytes() const;
    QString formattedTotalApplicationSize() const;
    QString lastError() const;

private:
    qint64 directorySizeBytes(const QString &path) const;
    QString formatBytes(qint64 bytes) const;

    mutable QString m_lastError;
};
