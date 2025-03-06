#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

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
#include <QFile>

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

protected:
    void closeEvent(QCloseEvent *event) override; //triggers when the app is closed
    void changeEvent(QEvent *event) override; //triggers when the app is minimized

private slots:
    void playPause();
    void startDiscovery();
    void appendDevice(const QBluetoothDeviceInfo &);
    void connectSelectedDevice();

    void saveInitData();
    void loadInitData();
    void showFromTray();
    void exitApp();

private:
    StartupHelp *startupHelp;
    Ui::PhoneAudioLink *ui;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothAddress savedDeviceAddress;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    bool maximizeBluetoothCompatability, startMinimized, connectAutomatically;

    QString stringifyUuids(QList<QBluetoothUuid>);

};
#endif // PHONEAUDIOLINK_H
