#ifndef BLUETOOTHA2DPSINK_H
#define BLUETOOTHA2DPSINK_H

#include <QObject>
#include <QString>
#include <QDebug>

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <wrl/client.h>
    #include <winrt/Windows.Foundation.h>
    #include <winrt/Windows.Media.Audio.h>
    #include <winrt/Windows.Devices.Enumeration.h>

    namespace winrt {
        using namespace Windows::Foundation;
        using namespace Windows::Media::Audio;
        using namespace Windows::Devices::Enumeration;
    }
#endif

class BluetoothA2DPSink : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothA2DPSink(QObject *parent = nullptr);
    ~BluetoothA2DPSink() override;

    // Start discovering A2DP-capable devices
    void startDeviceDiscovery();

    // Stop device discovery
    void stopDeviceDiscovery();

    // Enable A2DP sink for a specific device
    bool enableSink(const QString &deviceId);

    // Open the audio connection (starts audio streaming)
    bool openConnection();

    // Release/close the connection
    void releaseConnection();

    // Check current streaming status
    bool isStreaming() const;

    // Send media control commands
    void sendPlayPause();
    void sendNext();
    void sendPrevious();
    void sendStop();

signals:
    void deviceDiscovered(const QString &deviceId, const QString &deviceName);
    void discoveryCompleted();
    void sinkEnabled();
    void connectionOpened();
    void connectionClosed();
    void connectionError(const QString &error);
    void stateChanged(const QString &state);

private:
    void initializeWinRT();
    void cleanupWinRT();

#ifdef Q_OS_WIN
    winrt::fire_and_forget enableSinkAsync(std::wstring deviceId);
    winrt::fire_and_forget openConnectionAsync();
    void onConnectionStateChanged(winrt::AudioPlaybackConnection sender, winrt::IInspectable args);
    void sendMediaKey(DWORD vkCode);
    void onDeviceAdded(winrt::DeviceWatcher sender, winrt::DeviceInformation device);
    void onDeviceEnumerationCompleted(winrt::DeviceWatcher sender, winrt::IInspectable args);

    winrt::AudioPlaybackConnection m_connection{nullptr};
    winrt::DeviceWatcher m_deviceWatcher{nullptr};
    winrt::event_token m_stateChangedToken;
    winrt::event_token m_deviceAddedToken;
    winrt::event_token m_enumerationCompletedToken;
#endif

    bool m_winrtInitialized;
    bool m_ownsApartment;
    bool m_isStreaming;
    QString m_currentDeviceId;
};

#endif // BLUETOOTHA2DPSINK_H
