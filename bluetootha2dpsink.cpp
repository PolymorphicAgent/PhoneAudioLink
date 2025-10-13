#include "bluetootha2dpsink.h"
#include <QMetaObject>

#ifdef Q_OS_WIN
    using namespace winrt;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Media::Audio;
    using namespace winrt::Windows::Devices::Enumeration;
#endif

BluetoothA2DPSink::BluetoothA2DPSink(QObject *parent)
    : QObject(parent)
    , m_winrtInitialized(false)
    , m_ownsApartment(false)
    , m_isStreaming(false)
{
    qDebug() << "Initializing BluetoothA2DPSink...";
    initializeWinRT();
}

BluetoothA2DPSink::~BluetoothA2DPSink()
{
    cleanupWinRT();
}

void BluetoothA2DPSink::initializeWinRT()
{
#ifdef Q_OS_WIN
    try {
        // Try to initialize as Single-Threaded Apartment
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        m_winrtInitialized = true;
        m_ownsApartment = true;
        qDebug() << "WinRT initialized successfully (STA)";
    }
    catch (const winrt::hresult_error &ex) {
        HRESULT hr = ex.code();

        // RPC_E_CHANGED_MODE means COM already initialized - that's fine
        if (hr == RPC_E_CHANGED_MODE) {
            qDebug() << "COM already initialized, using existing apartment";
            m_winrtInitialized = true;
            m_ownsApartment = false;
        }
        else {
            qWarning() << "Failed to initialize WinRT:"
                       << QString::fromWCharArray(ex.message().c_str())
                       << "HRESULT:" << QString::number(hr, 16);
            m_winrtInitialized = false;
            m_ownsApartment = false;
        }
    }
#else
    qWarning() << "A2DP Sink is only supported on Windows 10 2004+";
#endif
}

void BluetoothA2DPSink::cleanupWinRT()
{
#ifdef Q_OS_WIN
    try {
        // Stop device watcher if running
        if (m_deviceWatcher) {
            if (m_deviceAddedToken.value != 0) {
                m_deviceWatcher.Added(m_deviceAddedToken);
                m_deviceAddedToken = {};
            }
            if (m_enumerationCompletedToken.value != 0) {
                m_deviceWatcher.EnumerationCompleted(m_enumerationCompletedToken);
                m_enumerationCompletedToken = {};
            }

            if (m_deviceWatcher.Status() == DeviceWatcherStatus::Started ||
                m_deviceWatcher.Status() == DeviceWatcherStatus::EnumerationCompleted) {
                m_deviceWatcher.Stop();
            }
            m_deviceWatcher = nullptr;
        }

        if (m_connection) {
            if (m_stateChangedToken.value != 0) {
                m_connection.StateChanged(m_stateChangedToken);
                m_stateChangedToken = {};
            }

            // Close and dispose
            if (m_connection.State() == AudioPlaybackConnectionState::Opened) {
                m_connection.Close();
            }
            m_connection = nullptr;
        }

        if (m_winrtInitialized && m_ownsApartment) {
            winrt::uninit_apartment();
            qDebug() << "WinRT apartment uninitialized";
        }

        m_winrtInitialized = false;
        m_ownsApartment = false;
    }
    catch (const winrt::hresult_error &ex) {
        qWarning() << "Error during cleanup:"
                   << QString::fromWCharArray(ex.message().c_str());
    }
#endif
}

void BluetoothA2DPSink::startDeviceDiscovery()
{
#ifdef Q_OS_WIN
    if (!m_winrtInitialized) {
        emit connectionError("WinRT not initialized");
        return;
    }

    try {
        qDebug() << "Starting device discovery for A2DP-capable devices...";

        // Get selector for devices that support AudioPlaybackConnection
        auto selector = AudioPlaybackConnection::GetDeviceSelector();
        qDebug() << "Device selector:" << QString::fromWCharArray(selector.c_str());

        // Create device watcher
        m_deviceWatcher = DeviceInformation::CreateWatcher(selector);

        // Register for device added events
        m_deviceAddedToken = m_deviceWatcher.Added(
            [this](DeviceWatcher sender, DeviceInformation device) {
                onDeviceAdded(sender, device);
            }
            );

        // Register for enumeration completed
        m_enumerationCompletedToken = m_deviceWatcher.EnumerationCompleted(
            [this](DeviceWatcher sender, winrt::IInspectable args) {
                onDeviceEnumerationCompleted(sender, args);
            }
            );

        // Start watching
        m_deviceWatcher.Start();
        qDebug() << "Device watcher started";
    }
    catch (const winrt::hresult_error &ex) {
        QString error = QString::fromWCharArray(ex.message().c_str());
        qWarning() << "Error starting device discovery:" << error;
        emit connectionError(error);
    }
#else
    emit connectionError("Device discovery only supported on Windows 10 2004+");
#endif
}

void BluetoothA2DPSink::stopDeviceDiscovery()
{
#ifdef Q_OS_WIN
    if (m_deviceWatcher) {
        try {
            if (m_deviceWatcher.Status() == DeviceWatcherStatus::Started ||
                m_deviceWatcher.Status() == DeviceWatcherStatus::EnumerationCompleted) {
                m_deviceWatcher.Stop();
                qDebug() << "Device watcher stopped";
            }
        }
        catch (const winrt::hresult_error &ex) {
            qWarning() << "Error stopping device watcher:"
                       << QString::fromWCharArray(ex.message().c_str());
        }
    }
#endif
}

#ifdef Q_OS_WIN
void BluetoothA2DPSink::onDeviceAdded(winrt::DeviceWatcher sender, winrt::DeviceInformation device)
{
    Q_UNUSED(sender);

    QString deviceId = QString::fromWCharArray(device.Id().c_str());
    QString deviceName = QString::fromWCharArray(device.Name().c_str());

    qDebug() << "Found A2DP device:" << deviceName << "ID:" << deviceId;

    // Emit on Qt thread
    QMetaObject::invokeMethod(this, [this, deviceId, deviceName]() {
        emit deviceDiscovered(deviceId, deviceName);
    }, Qt::QueuedConnection);
}

void BluetoothA2DPSink::onDeviceEnumerationCompleted(winrt::DeviceWatcher sender, winrt::IInspectable args)
{
    Q_UNUSED(sender);
    Q_UNUSED(args);

    qDebug() << "Device enumeration completed";

    QMetaObject::invokeMethod(this, [this]() {
        emit discoveryCompleted();
    }, Qt::QueuedConnection);
}
#endif

bool BluetoothA2DPSink::enableSink(const QString &deviceId)
{
#ifdef Q_OS_WIN
    if (!m_winrtInitialized) {
        emit connectionError("WinRT not initialized");
        return false;
    }

    // Release any existing connection first
    releaseConnection();

    m_currentDeviceId = deviceId;

    qDebug() << "Enabling A2DP sink for device:" << deviceId;

    // Convert QString to std::wstring
    std::wstring wDeviceId = deviceId.toStdWString();

    // Start async operation
    enableSinkAsync(wDeviceId);

    return true;
#else
    emit connectionError("A2DP Sink only supported on Windows 10 2004+");
    return false;
#endif
}

#ifdef Q_OS_WIN
winrt::fire_and_forget BluetoothA2DPSink::enableSinkAsync(std::wstring deviceId)
{
    try {
        qDebug() << "Creating AudioPlaybackConnection...";

        // Create the AudioPlaybackConnection for this device
        m_connection = AudioPlaybackConnection::TryCreateFromId(deviceId);

        if (!m_connection) {
            QMetaObject::invokeMethod(this, [this]() {
                emit connectionError("Failed to create AudioPlaybackConnection");
            }, Qt::QueuedConnection);
            co_return;
        }

        qDebug() << "AudioPlaybackConnection created successfully";

        // Register for state change events
        m_stateChangedToken = m_connection.StateChanged(
            [this](AudioPlaybackConnection sender, winrt::IInspectable args) {
                onConnectionStateChanged(sender, args);
            }
            );

        // Start the connection (enables incoming audio)
        qDebug() << "Starting AudioPlaybackConnection...";
        co_await m_connection.StartAsync();

        qDebug() << "AudioPlaybackConnection started successfully";

        // Notify on Qt thread
        QMetaObject::invokeMethod(this, [this]() {
            emit sinkEnabled();
            emit stateChanged("Sink Enabled - Ready to Connect");
        }, Qt::QueuedConnection);
    }
    catch (const winrt::hresult_error &ex) {
        QString error = QString::fromWCharArray(ex.message().c_str());
        qWarning() << "Error enabling sink:" << error;

        QMetaObject::invokeMethod(this, [this, error]() {
            emit connectionError(error);
        }, Qt::QueuedConnection);
    }
}

winrt::fire_and_forget BluetoothA2DPSink::openConnectionAsync()
{
    try {
        if (!m_connection) {
            QMetaObject::invokeMethod(this, [this]() {
                emit connectionError("No connection to open");
            }, Qt::QueuedConnection);
            co_return;
        }

        qDebug() << "Opening audio connection...";

        // Open the connection (audio starts flowing)
        auto result = co_await m_connection.OpenAsync();

        if (result.Status() == AudioPlaybackConnectionOpenResultStatus::Success) {
            qDebug() << "Audio connection opened successfully";
            m_isStreaming = true;

            QMetaObject::invokeMethod(this, [this]() {
                emit connectionOpened();
                emit stateChanged("Connected - Audio Streaming");
            }, Qt::QueuedConnection);
        }
        else {
            QString error = "Failed to open connection: ";
            switch (result.Status()) {
            case AudioPlaybackConnectionOpenResultStatus::RequestTimedOut:
                error += "Request timed out";
                break;
            case AudioPlaybackConnectionOpenResultStatus::DeniedBySystem:
                error += "Denied by system";
                break;
            case AudioPlaybackConnectionOpenResultStatus::UnknownFailure:
                error += "Unknown failure";
                break;
            default:
                error += "Unknown status";
                break;
            }

            qWarning() << error;

            QMetaObject::invokeMethod(this, [this, error]() {
                emit connectionError(error);
            }, Qt::QueuedConnection);
        }
    }
    catch (const winrt::hresult_error &ex) {
        QString error = QString::fromWCharArray(ex.message().c_str());
        qWarning() << "Error opening connection:" << error;

        QMetaObject::invokeMethod(this, [this, error]() {
            emit connectionError(error);
        }, Qt::QueuedConnection);
    }
}

void BluetoothA2DPSink::onConnectionStateChanged(
    winrt::AudioPlaybackConnection sender,
    winrt::IInspectable args)
{
    Q_UNUSED(args);

    auto state = sender.State();

    QMetaObject::invokeMethod(this, [this, state]() {
        QString stateStr;

        switch (state) {
        case AudioPlaybackConnectionState::Closed:
            stateStr = "Closed";
            m_isStreaming = false;
            emit connectionClosed();
            break;

        case AudioPlaybackConnectionState::Opened:
            stateStr = "Opened";
            m_isStreaming = true;
            emit connectionOpened();
            break;

        default:
            stateStr = "Unknown";
            break;
        }

        qDebug() << "Connection state changed:" << stateStr;
        emit stateChanged(stateStr);

    }, Qt::QueuedConnection);
}
#endif

bool BluetoothA2DPSink::openConnection()
{
#ifdef Q_OS_WIN
    if (!m_winrtInitialized) {
        emit connectionError("WinRT not initialized");
        return false;
    }

    if (!m_connection) {
        emit connectionError("Sink not enabled - call enableSink() first");
        return false;
    }

    qDebug() << "Requesting to open connection...";

    // Start async operation
    openConnectionAsync();

    return true;
#else
    emit connectionError("A2DP Sink only supported on Windows 10 2004+");
    return false;
#endif
}

void BluetoothA2DPSink::releaseConnection()
{
#ifdef Q_OS_WIN
    if (m_connection) {
        try {
            qDebug() << "Releasing A2DP connection...";

            // Unregister state changed event
            if (m_stateChangedToken.value != 0) {
                m_connection.StateChanged(m_stateChangedToken);
                m_stateChangedToken = {};
            }

            // Close if open
            if (m_connection.State() == AudioPlaybackConnectionState::Opened) {
                m_connection.Close();
            }

            m_connection = nullptr;
            m_isStreaming = false;

            emit connectionClosed();
            emit stateChanged("Disconnected");

            qDebug() << "Connection released successfully";
        }
        catch (const winrt::hresult_error &ex) {
            qWarning() << "Error releasing connection:"
                       << QString::fromWCharArray(ex.message().c_str());
        }
    }
#endif
}

bool BluetoothA2DPSink::isStreaming() const
{
    return m_isStreaming;
}

// Media control functions using SendInput
void BluetoothA2DPSink::sendPlayPause()
{
#ifdef Q_OS_WIN
    sendMediaKey(VK_MEDIA_PLAY_PAUSE);
    qDebug() << "Sent Play/Pause command";
#endif
}

void BluetoothA2DPSink::sendNext()
{
#ifdef Q_OS_WIN
    sendMediaKey(VK_MEDIA_NEXT_TRACK);
    qDebug() << "Sent Next Track command";
#endif
}

void BluetoothA2DPSink::sendPrevious()
{
#ifdef Q_OS_WIN
    sendMediaKey(VK_MEDIA_PREV_TRACK);
    qDebug() << "Sent Previous Track command";
#endif
}

void BluetoothA2DPSink::sendStop()
{
#ifdef Q_OS_WIN
    sendMediaKey(VK_MEDIA_STOP);
    qDebug() << "Sent Stop command";
#endif
}

#ifdef Q_OS_WIN
void BluetoothA2DPSink::sendMediaKey(DWORD vkCode)
{
    INPUT inputs[2] = {};

    // Key down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;

    // Key up
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT sent = SendInput(2, inputs, sizeof(INPUT));

    if (sent != 2) {
        qWarning() << "Failed to send media key:" << vkCode
                   << "Error:" << GetLastError();
    }
}
#endif
