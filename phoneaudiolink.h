#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

#include "startuphelp.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QMainWindow>

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

private slots:
    void playPause();
    void startDiscovery();
    void appendDevice(const QBluetoothDeviceInfo &);
    void connectSelectedDevice();

private:
    StartupHelp *startupHelp;
    Ui::PhoneAudioLink *ui;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    bool maximizeBluetoothCompatability;

};
#endif // PHONEAUDIOLINK_H
