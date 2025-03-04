#ifndef DEVICEPICKERCONTROLLER_H
#define DEVICEPICKERCONTROLLER_H

#include <QObject>
#include <QString>

#ifdef Q_OS_WIN
    #include <Windows.h>

    // Include WinRT headers
    #include <winrt/Windows.Foundation.h>
    #include <winrt/Windows.Devices.Enumeration.h>
    #include <winrt/Windows.UI.Popups.h>
#endif //Q_OS_WIN

class DevicePickerController : public QObject
{
    Q_OBJECT
public:
    explicit DevicePickerController(QObject *parent = nullptr);

signals:
};

#endif // DEVICEPICKERCONTROLLER_H
