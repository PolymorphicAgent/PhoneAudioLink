#include "a2dpstreamer.h"

#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QDebug>

A2DPStreamer::A2DPStreamer(QObject *parent)
    : QObject(parent),
    m_socket(nullptr),
    m_audioSink(nullptr),
    m_audioDevice(nullptr),
    m_isPlaying(false)
{
}

A2DPStreamer::~A2DPStreamer()
{
    disconnectDevice();
}

bool A2DPStreamer::connectToDevice(const QBluetoothDeviceInfo &deviceInfo)
{
    // Clean up any existing connection.
    if (m_socket) {
        disconnectDevice();
    }

    // Create a Bluetooth socket (using RFCOMM protocol).
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_socket, &QBluetoothSocket::connected, this, &A2DPStreamer::onSocketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &A2DPStreamer::onSocketDisconnected);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &A2DPStreamer::onReadyRead);

    // Initiate connection.
    // (Adjust the service UUID as needed – for example, QBluetoothUuid::AudioSink.)
    m_socket->connectToService(deviceInfo.address(), QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::AudioSink));
    return true;
}


void A2DPStreamer::disconnectDevice()
{
    if (m_socket) {
        if (m_socket->isOpen())
            m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    if (m_audioSink) {
        m_audioSink->stop();
        m_audioSink->deleteLater();
        m_audioSink = nullptr;
        m_audioDevice = nullptr;
    }
}

void A2DPStreamer::onSocketConnected()
{
    qDebug() << "Bluetooth socket connected.";

    // Setup the audio output.
    setupAudio();

    if (m_audioSink && m_socket) {
        // Start the audio sink and get a writable audio device.
        m_audioDevice = m_audioSink->start();
    }
}

void A2DPStreamer::onSocketDisconnected()
{
    qDebug() << "Bluetooth socket disconnected.";
    if (m_audioSink)
        m_audioSink->stop();
}

void A2DPStreamer::onReadyRead()
{
    if (!m_audioDevice)
        return;

    // Read available data from the Bluetooth socket and write it to the audio device.
    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        m_audioDevice->write(data);
    }
}

void A2DPStreamer::onAudioStateChanged(QAudio::State state)
{
    if (state == QAudio::IdleState) {
        // Playback may have finished.
    } else if (state == QAudio::StoppedState) {
        // Handle any errors if needed.
        // Note: In QAudioSink, error() might not be directly available.
    }
}

void A2DPStreamer::setupAudio()
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    // In Qt 6, set the sample format using QAudioFormat::setSampleFormat().
    format.setSampleFormat(QAudioFormat::Int16);

    // Get the default audio output device.
    QAudioDevice defaultOutput = QMediaDevices::defaultAudioOutput();
    if (!defaultOutput.isFormatSupported(format)) {
        qWarning() << "Default format not supported. Using preferred format.";
        format = defaultOutput.preferredFormat();
    }

    m_audioSink = new QAudioSink(defaultOutput, format, this);
    connect(m_audioSink, &QAudioSink::stateChanged,
            this, &A2DPStreamer::onAudioStateChanged);
}

void A2DPStreamer::sendCommand(const QString &command)
{
    if (m_socket && m_socket->isOpen()) {
        QByteArray data = command.toUtf8();
        m_socket->write(data);
        // QBluetoothSocket in Qt 6 may not offer flush(); use waitForBytesWritten if necessary.
        m_socket->waitForBytesWritten(100);
    } else {
        qWarning() << "Socket not connected. Cannot send command:" << command;
    }
}

void A2DPStreamer::playPause()
{
    // Toggle play/pause command.
    if (m_isPlaying) {
        sendCommand("PAUSE");
        m_isPlaying = false;
    } else {
        sendCommand("PLAY");
        m_isPlaying = true;
    }
}

void A2DPStreamer::forward()
{
    sendCommand("FORWARD");
}

void A2DPStreamer::backward()
{
    sendCommand("BACKWARD");
}
