#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QNetworkAccessManager>
#include <QVersionNumber>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    void checkForUpdates(const QString &currentVersion);

signals:
    void updateAvailable(const QString &newVersion, const QString &releaseNotesUrl);
    void noUpdateAvailable();
    void checkFailed(const QString &error);

private slots:
    void onVersionCheckFinished();

private:
    QNetworkAccessManager *m_networkManager;
    QString m_currentVersion;

    // points to my repository
    const QString UPDATE_CHECK_URL = "https://repo.polimorph.dev/PhoneAudioLink/latest-version.json";
};

#endif // UPDATECHECKER_H
