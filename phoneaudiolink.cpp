#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{

    audioSinkUuid = new QBluetoothUuid(QStringLiteral("0000110B-0000-1000-8000-00805F9B34FB"));

    ui->setupUi(this);
    ui->dcLabel->setStyleSheet("QLabel { color : red; }");

    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512.png"));
    }
    else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512-white.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512-white.png"));
    }


    connect(ui->playPause, SIGNAL(pressed()), this, SLOT(     playPause()));
    connect(ui->refresh  , SIGNAL(pressed()), this, SLOT(startDiscovery()));

    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &PhoneAudioLink::appendDevice);

    startDiscovery(); //automatically look for devices
}

PhoneAudioLink::~PhoneAudioLink() {
    delete ui;
    delete discoveryAgent;
}

void PhoneAudioLink::playPause() {
    ui->playPause->toggleState();
}

void PhoneAudioLink::startDiscovery() {
    ui->deviceComboBox->clear();
    discoveryAgent->stop();
    discoveryAgent->start();
}

void PhoneAudioLink::appendDevice(const QBluetoothDeviceInfo &device) {
    // Filter: only add devices that advertise the Audio Sink service
    qDebug()<<"discovered device";
    qDebug()<<"\tName: "               <<device.name();
    qDebug()<<"\tMajor Device Class: " <<device.majorDeviceClass();
    qDebug()<<"\tMinor Device Class: " <<device.minorDeviceClass();
    qDebug()<<"\tCore Configurations: "<<device.coreConfigurations().toInt();
    if(device.name().startsWith("Bluetooth")&&device.name().contains(":")) {
        qDebug()<<"FILTERED OUT";
        return;
    }
    ui->deviceComboBox->addItem(device.name(), QVariant::fromValue(device));
}

void PhoneAudioLink::connectSelectedDevice() {
    //get the device info from the ComboBox
    auto device = ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>();
    QBluetoothLocalDevice localDevice;
    //pair if not already paired.
    if (localDevice.hostMode() != QBluetoothLocalDevice::HostConnectable) {
        localDevice.powerOn();
    }
    if (localDevice.pairingStatus(device.address()) != QBluetoothLocalDevice::Paired) {
        localDevice.requestPairing(device.address(), QBluetoothLocalDevice::Paired);
    }
    // Once paired, the OS should handle A2DP routing.
}

