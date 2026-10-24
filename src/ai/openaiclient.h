#pragma once

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrl>

class OpenAiClient : public QObject
{
    Q_OBJECT

public:
    explicit OpenAiClient(QObject *parent = nullptr);

    void setBaseUrl(const QUrl &baseUrl);
    void createResponseStream(const QString &apiKey, const QJsonObject &payload);
    void createResponse(const QString &apiKey, const QJsonObject &payload);
    void createChatCompletion(const QString &apiKey, const QJsonObject &payload);

signals:
    void responseDelta(const QString &delta);
    void responseCompleted(const QJsonObject &response);
    void requestFailed(const QString &message);

private:
    QNetworkAccessManager m_manager;
    QUrl m_baseUrl = QUrl("https://api.openai.com/v1");
};
