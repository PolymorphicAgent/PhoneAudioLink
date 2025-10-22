#include "audiosessionmanager.h"

#include <QCoreApplication>
#include <combaseapi.h>
#include <QDebug>

AudioSessionManager::AudioSessionManager(QObject *parent)
    : QObject(parent), m_sessionManager(nullptr), m_sessionControl(nullptr)
{
    CoInitialize(nullptr);
}

AudioSessionManager::~AudioSessionManager()
{
    if (m_sessionControl) {
        m_sessionControl->Release();
    }
    if (m_sessionManager) {
        m_sessionManager->Release();
    }
    CoUninitialize();
}

bool AudioSessionManager::findPhoneAudioSession()
{
    // HRESULT hr;

    // // Get the default audio endpoint
    // IMMDeviceEnumerator *deviceEnumerator = nullptr;
    // hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
    //                       CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
    //                       (void**)&deviceEnumerator);
    // if (FAILED(hr)) {
    //     qWarning() << "Failed to create device enumerator:" << hr;
    //     return false;
    // }

    // IMMDevice *device = nullptr;
    // hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    // deviceEnumerator->Release();

    // if (FAILED(hr)) {
    //     qWarning() << "Failed to get default audio endpoint:" << hr;
    //     return false;
    // }

    // // Get the session manager
    // hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER,
    //                       nullptr, (void**)&m_sessionManager);
    // device->Release();

    // if (FAILED(hr)) {
    //     qWarning() << "Failed to activate session manager:" << hr;
    //     return false;
    // }

    // // Enumerate sessions to find ours
    // IAudioSessionEnumerator *sessionEnumerator = nullptr;
    // hr = m_sessionManager->GetSessionEnumerator(&sessionEnumerator);
    // if (FAILED(hr)) {
    //     qWarning() << "Failed to get session enumerator:" << hr;
    //     return false;
    // }

    // int sessionCount = 0;
    // sessionEnumerator->GetCount(&sessionCount);

    // DWORD currentProcessId = GetCurrentProcessId();

    // for (int i = 0; i < sessionCount; i++) {
    //     IAudioSessionControl *sessionControl = nullptr;
    //     hr = sessionEnumerator->GetSession(i, &sessionControl);

    //     if (SUCCEEDED(hr)) {
    //         IAudioSessionControl2 *sessionControl2 = nullptr;
    //         hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2),
    //                                             (void**)&sessionControl2);
    //         sessionControl->Release();

    //         if (SUCCEEDED(hr)) {
    //             DWORD processId = 0;
    //             sessionControl2->GetProcessId(&processId);

    //             if (processId == currentProcessId) {
    //                 m_sessionControl = sessionControl2;
    //                 sessionEnumerator->Release();
    //                 return true;
    //             }
    //             sessionControl2->Release();
    //         }
    //     }
    // }

    // sessionEnumerator->Release();
    // qWarning() << "Could not find audio session for current process";
    // return false;
    HRESULT hr;

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                          CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
                          (void**)&deviceEnumerator);
    if (FAILED(hr)) {
        qWarning() << "Failed to create device enumerator:" << hr;
        return false;
    }

    DWORD currentProcessId = GetCurrentProcessId();
    qDebug() << "Current process ID:" << currentProcessId;

    // Try both capture (recording) and render (playback) devices
    // A2DP Sink appears as capture since audio comes FROM the phone
    EDataFlow dataFlows[] = { eCapture, eRender };

    for (int flowIdx = 0; flowIdx < 2; flowIdx++) {
        EDataFlow dataFlow = dataFlows[flowIdx];
        qDebug() << "Searching" << (dataFlow == eCapture ? "CAPTURE" : "RENDER") << "devices...";

        IMMDevice *device = nullptr;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &device);

        if (FAILED(hr)) {
            qWarning() << "Failed to get default audio endpoint for flow" << dataFlow << ":" << hr;
            continue;
        }

        // Get the session manager
        IAudioSessionManager2 *sessionManager = nullptr;
        hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER,
                              nullptr, (void**)&sessionManager);
        device->Release();

        if (FAILED(hr)) {
            qWarning() << "Failed to activate session manager:" << hr;
            continue;
        }

        // Enumerate sessions
        IAudioSessionEnumerator *sessionEnumerator = nullptr;
        hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
        if (FAILED(hr)) {
            qWarning() << "Failed to get session enumerator:" << hr;
            sessionManager->Release();
            continue;
        }

        int sessionCount = 0;
        sessionEnumerator->GetCount(&sessionCount);
        qDebug() << "Found" << sessionCount << "sessions";

        for (int i = 0; i < sessionCount; i++) {
            IAudioSessionControl *sessionControl = nullptr;
            hr = sessionEnumerator->GetSession(i, &sessionControl);

            if (SUCCEEDED(hr)) {
                IAudioSessionControl2 *sessionControl2 = nullptr;
                hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2),
                                                    (void**)&sessionControl2);
                sessionControl->Release();

                if (SUCCEEDED(hr)) {
                    DWORD processId = 0;
                    sessionControl2->GetProcessId(&processId);

                    LPWSTR displayName = nullptr;
                    sessionControl2->GetDisplayName(&displayName);
                    QString name = displayName ? QString::fromWCharArray(displayName) : QString("(no name)");
                    CoTaskMemFree(displayName);

                    qDebug() << "  Session" << i << "- PID:" << processId << "Name:" << name;

                    // Match by process ID OR by name containing "Microphone" or "A2DP"
                    if (processId == currentProcessId ||
                        name.contains("Microphone", Qt::CaseInsensitive) ||
                        name.contains("A2DP", Qt::CaseInsensitive) ||
                        name.contains("Phone Audio Link", Qt::CaseInsensitive)) {

                        qDebug() << "*** MATCHED session:" << name;
                        m_sessionControl = sessionControl2;
                        m_sessionManager = sessionManager;
                        sessionEnumerator->Release();
                        deviceEnumerator->Release();
                        return true;
                    }
                    sessionControl2->Release();
                }
            }
        }

        sessionEnumerator->Release();
        sessionManager->Release();
    }

    deviceEnumerator->Release();
    qWarning() << "Could not find audio session";
    return false;
}

bool AudioSessionManager::setSessionProperties(const QString &displayName, const QString &iconPath)
{
    // if (!m_sessionControl) {
    //     if (!findPhoneAudioSession()) {
    //         return false;
    //     }
    // }

    // if (!m_sessionControl) {
    //     return false;
    // }

    // // Set display name
    // HRESULT hr = m_sessionControl->SetDisplayName(
    //     reinterpret_cast<LPCWSTR>(displayName.utf16()), nullptr);

    // if (FAILED(hr)) {
    //     qWarning() << "Failed to set display name:" << hr;
    //     return false;
    // }

    // // Set icon path
    // hr = m_sessionControl->SetIconPath(
    //     reinterpret_cast<LPCWSTR>(iconPath.utf16()), nullptr);

    // if (FAILED(hr)) {
    //     qWarning() << "Failed to set icon path:" << hr;
    //     return false;
    // }

    // qDebug() << "Successfully set audio session properties";
    // return true;
    // Always try to find the session first
    if (m_sessionControl) {
        m_sessionControl->Release();
        m_sessionControl = nullptr;
    }
    if (m_sessionManager) {
        m_sessionManager->Release();
        m_sessionManager = nullptr;
    }

    if (!findPhoneAudioSession()) {
        return false;
    }

    if (!m_sessionControl) {
        return false;
    }

    qDebug() << "Setting display name to:" << displayName;
    qDebug() << "Setting icon path to:" << iconPath;

    // Set display name
    HRESULT hr = m_sessionControl->SetDisplayName(
        reinterpret_cast<LPCWSTR>(displayName.utf16()), nullptr);

    if (FAILED(hr)) {
        qWarning() << "Failed to set display name:" << QString::number(hr, 16);
        return false;
    }

    // Set icon path
    hr = m_sessionControl->SetIconPath(
        reinterpret_cast<LPCWSTR>(iconPath.utf16()), nullptr);

    if (FAILED(hr)) {
        qWarning() << "Failed to set icon path:" << QString::number(hr, 16);
        return false;
    }

    qDebug() << "Successfully set audio session properties!";
    return true;
}
