#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

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
    Ui::PhoneAudioLink *ui;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    const QBluetoothUuid *audioSinkUuid;

};
#endif // PHONEAUDIOLINK_H
