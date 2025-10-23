#include "updatechecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent),
    m_networkManager(new QNetworkAccessManager(this))
{}

UpdateChecker::~UpdateChecker()
{
    if (m_networkManager) {
        m_networkManager->deleteLater();
        m_networkManager = nullptr;
    }
}

void UpdateChecker::checkForUpdates(const QString &currentVersion)
{
    m_currentVersion = currentVersion;

    qDebug() << "Checking for updates. Current version:" << currentVersion;

    QNetworkRequest request((QUrl(UPDATE_CHECK_URL)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "PhoneAudioLink/" + currentVersion);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &UpdateChecker::onVersionCheckFinished);
}

void UpdateChecker::onVersionCheckFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Update check failed:" << reply->errorString();
        emit checkFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON response from update server";
        emit checkFailed("Invalid response from server");
        return;
    }

    QJsonObject obj = doc.object();
    QString remoteVersion = obj["version"].toString();
    QString releaseNotesUrl = obj["releaseNotesUrl"].toString();

    if (remoteVersion.isEmpty()) {
        qWarning() << "No version found in response";
        emit checkFailed("Invalid version data");
        return;
    }

    qDebug() << "Remote version:" << remoteVersion;

    // Compare versions
    QVersionNumber current = QVersionNumber::fromString(m_currentVersion);
    QVersionNumber remote = QVersionNumber::fromString(remoteVersion);

    if (remote > current) {
        qDebug() << "Update available:" << remoteVersion;
        emit updateAvailable(remoteVersion, releaseNotesUrl);
    } else {
        qDebug() << "No update available";
        emit noUpdateAvailable();
    }
}
