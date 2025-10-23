#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

#include "updatenotificationbar.h"
#include "audiosessionmanager.h"
#include "releasenotesdialog.h"
#include "bluetootha2dpsink.h"
#include "updatechecker.h"
#include "startuphelp.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QSystemTrayIcon>
#include <QJsonDocument>
#include <QMessageBox>
#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
#include <QTimer>
#include <QFile>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui {
class PhoneAudioLink;
}
QT_END_NAMESPACE

class PhoneAudioLink : public QMainWindow
{
    Q_OBJECT

public:
    PhoneAudioLink(QWidget *parent = nullptr);
    ~PhoneAudioLink();

    // Returns whether or not the program should be starting minimized
    bool getStartMinimized();

protected:
    void closeEvent(QCloseEvent *event) override; //triggers when the app is closed
    void changeEvent(QEvent *event) override; //triggers when the app is minimized
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void playPause();//is triggered when the play/pause button is pressed
    void startDiscovery();//starts the automatic discovery of bluetooth devices.
    void appendDevice(const QBluetoothDeviceInfo &);//is connected to the bluetooth discovery agent's deviceDiscovered slot
    void onA2DPDeviceDiscovered(const QString &deviceId, const QString &deviceName);
    void onA2DPDiscoveryCompleted();
    void connectSelectedDevice(); //triggers when the "connect" button is pressed
    void disconnect();
    void deviceComboChanged(int); //triggers when the deviceComboBox's index changes

    void saveInitData();//saves the json initialization configuration
    void loadInitData();//loads the json initialization configuration
    void showFromTray();//show the app from tray
    void updateTrayContext();
    void updateAutoConnectMenu();
    void exitApp();//saves init data, then exits the app

private:
    StartupHelp *startupHelp; //declare startup help dialog (instructs user on how to add the program to startup)
    Ui::PhoneAudioLink *ui; //the ui
    QBluetoothDeviceDiscoveryAgent *discoveryAgent; //the discovety agent
    AudioSessionManager *audioSessionManager; // for setting the app icon and name in windows volume mixer
    QBluetoothAddress savedDeviceAddress; //the address of the device from our json initialization configuration file
    QSystemTrayIcon *trayIcon; //the tray icon
    QMenu *trayMenu; //the tray context menu
    bool maximizeBluetoothCompatability, startMinimized, connectAutomatically;//these are loaded from our json initialization configuration file

    QString stringifyUuids(QList<QBluetoothUuid>); //for debugging purposes
    QString findDeviceName(const QBluetoothAddress&);

    QString connectedDevice; // keep track of the currently connected device

    BluetoothA2DPSink *audioSink;
    QList<QBluetoothDeviceInfo> discoveredDevices; //list of discovered devices
    QList<QAction*> trayDeviceActions, trayDeviceStartupActions, autoConnectMenuActions;

    // Map device names to Windows device IDs for A2DP
    QMap<QString, QString> deviceIdMap;

    // Track if we've shown connection notification (prevent duplicates), and whether or not the window is visible
    bool connectionNotificationShown, windowShown;

    // Version checking stuff
    UpdateChecker *updateChecker;
    UpdateNotificationBar *updateNotificationBar;
    QString pendingReleaseNotesUrl;
    QString pendingVersion;
    bool manuallyChecked = false;

private slots:
    void onUpdateAvailable(const QString &newVersion, const QString &releaseNotesUrl);
    void showReleaseNotes(const QString &releaseNotesUrl);
    void launchMaintenanceTool();
};
#endif // PHONEAUDIOLINK_H
