#ifndef A2DPSTREAMER_H
#define A2DPSTREAMER_H

#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QAudioFormat>
#include <QAudioSink>
#include <QObject>

class A2DPStreamer : public QObject
{
    Q_OBJECT
public:
    explicit A2DPStreamer(QObject *parent = nullptr);
    ~A2DPStreamer();

    // Connects to the selected Bluetooth device.
    bool connectToDevice(const QBluetoothDeviceInfo &deviceInfo);
    void disconnectDevice();

public slots:
    // Tells the Bluetooth device to play or pause.
    void playPause();
    // Tells the Bluetooth device to skip forwards.
    void forward();
    // Tells the Bluetooth device to skip backwards.
    void backward();

private slots:
    // Internal slots to manage the socket and audio states.
    void onSocketConnected();
    void onSocketDisconnected();
    void onReadyRead();
    void onAudioStateChanged(QAudio::State state);
    void onSocketError(QBluetoothSocket::SocketError error);
    void onServiceDiscovered(const QBluetoothServiceInfo &info);
    void onServiceDiscoveryFinished();

private:
    // Helper to send a command to the Bluetooth device.
    void sendCommand(const QString &command);
    // Sets up the audio output with a suitable format.
    void setupAudio();

    QBluetoothSocket *m_socket;
    QAudioSink *m_audioSink;         // New audio sink for Qt 6
    QIODevice *m_audioDevice;        // Device returned by m_audioSink->start() to write audio data
    bool m_isPlaying;

    // Members for service discovery.
    QBluetoothServiceDiscoveryAgent *m_serviceDiscoveryAgent;
    QBluetoothDeviceInfo m_pendingDevice;
};

#endif // A2DPSTREAMER_H
