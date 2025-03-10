#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

#include "startuphelp.h"
#include "A2DPStreamer.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QSystemTrayIcon>
#include <QJsonDocument>
#include <QMessageBox>
#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
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

    //returns whether or not the program should be starting minimized
    bool getStartMinimized();

protected:
    void closeEvent(QCloseEvent *event) override; //triggers when the app is closed
    void changeEvent(QEvent *event) override; //triggers when the app is minimized

private slots:
    void playPause();//is triggered when the play/pause button is pressed
    void startDiscovery();//starts the automatic discovery of bluetooth devices.
    void appendDevice(const QBluetoothDeviceInfo &);//is connected to the bluetooth discovery agent's deviceDiscovered slot
    void connectSelectedDevice(); //triggers when the "connect" button is pressed
    void deviceComboChanged(int); //triggers when the deviceComboBox's index changes

    void saveInitData();//saves the json initialization configuration
    void loadInitData();//loads the json initialization configuration
    void showFromTray();//show the app from tray
    void exitApp();//saves init data, then exits the app

private:
    StartupHelp *startupHelp; //declare startup help dialog (instructs user on how to add the program to startup)
    Ui::PhoneAudioLink *ui; //the ui
    QBluetoothDeviceDiscoveryAgent *discoveryAgent; //the discovety agent
    QBluetoothAddress savedDeviceAddress; //the address of the device from our json initialization configuration file
    QSystemTrayIcon *trayIcon; //the tray icon
    QMenu *trayMenu; //the tray context menu
    bool maximizeBluetoothCompatability, startMinimized, connectAutomatically;//these are loaded from our json initialization configuration file

    QString stringifyUuids(QList<QBluetoothUuid>); //for debugging purposes

    //members for A2DP streaming
    A2DPStreamer *a2dpStreamer; //our streaming handler
    QList<QBluetoothDeviceInfo> discoveredDevices; //list of discovered devices
};
#endif // PHONEAUDIOLINK_H
