#ifndef AUDIOSESSIONMANAGER_H
#define AUDIOSESSIONMANAGER_H

#include <QObject>
#include <QString>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioclient.h>

class AudioSessionManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioSessionManager(QObject *parent = nullptr);
    ~AudioSessionManager();

    bool setSessionProperties(const QString &displayName, const QString &iconPath);

private:
    bool findPhoneAudioSession();

    IAudioSessionManager2 *m_sessionManager;
    IAudioSessionControl2 *m_sessionControl;
};

#endif // AUDIOSESSIONMANAGER_H
