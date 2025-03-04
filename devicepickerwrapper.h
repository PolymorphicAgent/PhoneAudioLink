#ifndef DEVICEPICKERWRAPPER_H
#define DEVICEPICKERWRAPPER_H

#include <QObject>
#include <QString>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shobjidl_core.h>
#include <d2d1_3.h>
#include <shlwapi.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Media.Audio.h>
#include <winrt/Windows.Devices.Enumeration.h>
#endif //Q_OS_WIN

class DevicePickerWrapper : public QObject
{
    Q_OBJECT
public:
    // Pass the parent window handle (HWND) so that the picker can be attached.
    explicit DevicePickerWrapper(HWND parentHwnd, QObject *parent = nullptr);
    ~DevicePickerWrapper();

    // Call this to show the picker.
    void showPicker();

signals:
    // Emitted when a device is selected.
    void deviceSelected(const QString &deviceId);
    // Emitted when a device disconnect action is triggered.
    void deviceDisconnected(const QString &deviceId);

private:
#ifdef Q_OS_WIN
         // The WinRT DevicePicker instance.
    winrt::Windows::Devices::Enumeration::DevicePicker m_devicePicker{ nullptr };
#endif
    HWND m_parentHwnd;
};

#endif // DEVICEPICKERWRAPPER_H
