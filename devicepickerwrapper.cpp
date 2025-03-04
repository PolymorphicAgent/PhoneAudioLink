#include "devicepickerwrapper.h"

DevicePickerWrapper::DevicePickerWrapper(HWND parentHwnd, QObject *parent)
    : QObject(parent)
    , m_parentHwnd(parentHwnd)
{
#ifdef Q_OS_WIN
    // Create and initialize the DevicePicker.
    m_devicePicker = winrt::Windows::Devices::Enumeration::DevicePicker();
    auto initializeWithWindow = m_devicePicker.as<IInitializeWithWindow>();
    winrt::check_hresult(initializeWithWindow->Initialize(m_parentHwnd));

    // Append the filtering selector from AudioPlaybackConnection.
    // This line makes sure that only devices matching the selector (e.g., A2DP devices)
    // are shown in the picker.
    m_devicePicker.Filter().SupportedDeviceSelectors().Append(AudioPlaybackConnection::GetDeviceSelector());

    // Register a callback for when the picker is dismissed.
    m_devicePicker.DevicePickerDismissed([](auto const&, auto const&) {
        // Hide the window; adjust this behavior as needed.
        SetWindowPos(nullptr, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_HIDEWINDOW);
    });

    // Register a callback for device selection.
    m_devicePicker.DeviceSelected([this](auto const& sender, auto const& args) {
        auto device = args.SelectedDevice();
        // Emit our Qt signal with the device ID (converted to QString).
        emit deviceSelected(QString::fromStdWString(device.Id().c_str()));
        // Optionally, you can call your connection method here.
    });

    // Register a callback for disconnect button clicks.
    m_devicePicker.DisconnectButtonClicked([this](auto const& sender, auto const& args) {
        auto device = args.Device();
        emit deviceDisconnected(QString::fromStdWString(device.Id().c_str()));
        // Reset the device display status.
        sender.SetDisplayStatus(device, L"", winrt::Windows::Devices::Enumeration::DevicePickerDisplayStatusOptions::None);
    });
#endif
}

void DevicePickerWrapper::showPicker()
{
#ifdef Q_OS_WIN
    // Define a rectangle where the picker should appear.
    // Here we hard-code a rectangle; in a real application you might compute this
    // from a UI element's geometry.
    // RECT rect = { 100, 100, 400, 400 };
    // m_devicePicker.Show(rect, winrt::Windows::Devices::Enumeration::Placement::Above);
#endif
}
