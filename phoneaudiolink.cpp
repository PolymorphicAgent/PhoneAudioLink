#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{
    ui->setupUi(this);
    ui->dcLabel->setStyleSheet("QLabel { color : red; }");
    ui->menuAdvanced->setToolTipsVisible(true);

    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512.png"));
    }
    else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512-white.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512-white.png"));
    }

    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &PhoneAudioLink::appendDevice);

    startDiscovery(); //automatically look for devices

    //load initialization data from "init.json" if it exists
    loadInitData();

    //create a system tray icon
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/icon.ico"), this);

    //create the tray context menu
    trayMenu = new QMenu(this);
    QAction *restoreAction = new QAction("Show", this);
    QAction *quitAction = new QAction("Exit", this);

    //connect the tray context menu buttons to their respective actions
    connect(restoreAction, &QAction::triggered, this, &PhoneAudioLink::showFromTray);
    connect(quitAction, &QAction::triggered, this, &PhoneAudioLink::exitApp);

    //add the actions
    trayMenu->addAction(restoreAction);
    trayMenu->addAction(quitAction);

    //configure and show the tray icon
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    //connect tray icon clicked signal to showFromTray
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r){
        if(r == QSystemTrayIcon::ActivationReason::MiddleClick){
            this->exitApp();
            return;
        }
        else if(r == QSystemTrayIcon::ActivationReason::Trigger || r == QSystemTrayIcon::ActivationReason::DoubleClick){
            this->showFromTray();
            return;
        }
    });

    connect(ui->playPause, SIGNAL(pressed()), this, SLOT(     playPause()));
    connect(ui->refresh  , SIGNAL(pressed()), this, SLOT(startDiscovery()));

    ui->compatAction->setChecked(maximizeBluetoothCompatability);
    ui->connectStartupAction->setChecked(connectAutomatically);
    ui->startMinimizedAction->setChecked(startMinimized);

    connect(ui->compatAction, &QAction::triggered, this, [this](bool checked){
        maximizeBluetoothCompatability=checked;
    });

    connect(ui->connectStartupAction, &QAction::triggered, this, [this](bool checked){
        connectAutomatically=checked;
    });

    connect(ui->startMinimizedAction, &QAction::triggered, this, [this](bool checked){
        startMinimized=checked;
    });

    startupHelp = new StartupHelp(this);

    connect(ui->startOnLoginAction, &QAction::triggered, this, [this](){
        this->startupHelp->exec();
    });
}

PhoneAudioLink::~PhoneAudioLink() {
    delete ui;
    delete discoveryAgent;
}

//detect window minimize event
void PhoneAudioLink::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            saveInitData();
            hide();  //hide window if minimized
        }
    }
    QMainWindow::changeEvent(event); //call the super method
}

//show the window from tray
void PhoneAudioLink::showFromTray() {
    showNormal();
    activateWindow();
}

//exit the application
void PhoneAudioLink::exitApp() {
    saveInitData();
    trayIcon->hide();//hide the tray icon
    QApplication::quit();//call the super method
}

//handle the close event
void PhoneAudioLink::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    exitApp();
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
    //filter devices
    if(device.name().startsWith("Bluetooth") && device.name().contains(":")) {
        return;
    }
    qDebug()<<"discovered device";
    qDebug()<<"\tName: "               <<device.name();
    qDebug()<<"\tMajor, Minor Device Classes: " <<device.majorDeviceClass()<<device.minorDeviceClass();
    qDebug()<<"\tDevice address: "<<device.address();

    if(device.address() == savedDeviceAddress){
        qDebug()<<"Matched!";
        ui->deviceComboBox->addItem(device.name(), QVariant::fromValue(device));
        ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findData(QVariant::fromValue(device)));
        qDebug()<<"added!";
        return;
    }

    if(device.majorDeviceClass() == QBluetoothDeviceInfo::PhoneDevice){
        ui->deviceComboBox->addItem(device.name(), QVariant::fromValue(device));
        qDebug()<<"added!";
    }
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

//save initialization data
void PhoneAudioLink::saveInitData() {
    //initialize the file object
    QString fileName = "init.json";
    QJsonObject config;

    config["maximizeBluetoothCompatability"] = maximizeBluetoothCompatability;
    config["connectAutomatically"] = connectAutomatically;
    config["startMinimized"] = startMinimized;
    config["device"] = ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().address().toString();

    //write the file
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(config).toJson());
        file.close();
    }
    else {
        //alert the user of errors
        QMessageBox::critical(this, tr("Error"), tr("Failed to save initialization configuration file."));
    }

}

//load initialization data
void PhoneAudioLink::loadInitData(){
    //initialize file object
    QString initFilePath = "init.json";
    QFile initFile(initFilePath);
    bool err = false;

    //check if init.json exists
    if(initFile.exists()){
        //read the file
        initFile.open(QIODevice::ReadOnly);
        QJsonDocument doc = QJsonDocument::fromJson(initFile.readAll());
        initFile.close();
        QJsonObject initConfig = doc.object();

        //grab bluetooth compatability state
        if (initConfig.contains("maximizeBluetoothCompatability"))
            maximizeBluetoothCompatability = initConfig["maximizeBluetoothCompatability"].toBool();
        else {
            //alert user of incorrectly formatted configuration
            QMessageBox::critical(this, tr("Error: NoBoolBluetoothError"), tr("Failed to parse configuration file \'init.config\'"));
            err = true;
        }

        //grab automatic connection state
        if(initConfig.contains("connectAutomatically"))
            connectAutomatically = initConfig["connectAutomatically"].toBool();
        else {
            //alert user of incorrectly formatted configuration
            QMessageBox::critical(this, tr("Error: NoBoolConnectError"), tr("Failed to parse configuration file \'init.config\'"));
            err = true;
        }

        //grab start minimized state
        if(initConfig.contains("startMinimized"))
            startMinimized = initConfig["startMinimized"].toBool();
        else {
            //alert user of incorrectly formatted configuration
            QMessageBox::critical(this, tr("Error: NoStartupMinimizedError"), tr("Failed to parse configuration file \'init.config\'"));
            err = true;
        }

        //load the device
        if (initConfig.contains("device")) {
            savedDeviceAddress = QBluetoothAddress(initConfig["device"].toString());
        }
        else {
            //alert user of incorrectly formatted configuration
            QMessageBox::critical(this, tr("Error: NoDeviceError"), tr("Failed to parse configuration file \'init.config\'"));
            return;
        }
    }
    else{
        //if the initialization configuration doesn't exist, set initialization info to defaults
        maximizeBluetoothCompatability = false;
        connectAutomatically = false;
        startMinimized = false;
    }
    if(err)
        QMessageBox::critical(this, tr("Error: Initialization Configuration File Corrupted"), tr("Try deleting the file \'init.config\' and restarting the program. \nYour Initialization settings will be cleared."));
}

QString PhoneAudioLink::stringifyUuids(QList<QBluetoothUuid> l){
    QString result = "";
    for(QBluetoothUuid i:l){
        result+=i.toString()+", ";
    }
    return result;
}
