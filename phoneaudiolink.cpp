#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

#include <QRegularExpression>
#include <QTimer>

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
    , audioSink(nullptr)
{
    ui->setupUi(this);
    ui->dcLabel->setStyleSheet("QLabel { color : red; }");
    ui->menuAdvanced->setToolTipsVisible(true);

    // Set icons based on color scheme
    if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512.png"));
    }
    else if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        ui->forward->setIcon(QPixmap(":/icons/Iconparts/forward-512-white.png"));
        ui->back->setIcon(QPixmap(":/icons/Iconparts/back-512-white.png"));
    }

    // Create A2DP Sink manager
    audioSink = new BluetoothA2DPSink(this);

    // Connect A2DP device discovery signals
    connect(audioSink, &BluetoothA2DPSink::deviceDiscovered, this,
            &PhoneAudioLink::onA2DPDeviceDiscovered);
    connect(audioSink, &BluetoothA2DPSink::discoveryCompleted, this,
            &PhoneAudioLink::onA2DPDiscoveryCompleted);

    // Connect sink signals to UI updates
    connect(audioSink, &BluetoothA2DPSink::sinkEnabled, this, [this]() {
        ui->dcLabel->setText("Sink Enabled");
        ui->dcLabel->setStyleSheet("QLabel { color : orange; }");
        qDebug() << "A2DP Sink enabled, now open connection";

        // Automatically open the connection after enabling
        QTimer::singleShot(500, this, [this]() {
            audioSink->openConnection();
        });
    });

    connect(audioSink, &BluetoothA2DPSink::connectionOpened, this, [this]() {
        // Only show notification if we haven't already for this connection
        if (!connectionNotificationShown) {
            connectionNotificationShown = true;

            qDebug() << "Audio streaming active!";

            // Show notification once
            if (trayIcon) {
                trayIcon->showMessage("Audio Streaming",
                                      "Your phone is now streaming audio to this PC",
                                      QSystemTrayIcon::Information, 3000);
            }
        }

        ui->dcLabel->setText("Connected");
        ui->dcLabel->setStyleSheet("QLabel { color : green; }");
        ui->connect->setEnabled(false);
        ui->disconnect->setEnabled(true);
    });

    connect(audioSink, &BluetoothA2DPSink::connectionClosed, this, [this]() {
        // Reset notification flag so it shows again on next connection
        connectionNotificationShown = false;

        ui->dcLabel->setText("Disconnected!");
        ui->dcLabel->setStyleSheet("QLabel { color : red; }");
        ui->connect->setEnabled(true);
        ui->disconnect->setEnabled(false);
        qDebug() << "Audio streaming stopped";
    });

    connect(audioSink, &BluetoothA2DPSink::connectionError, this, [this](const QString &error) {
        QMessageBox::warning(this, tr("Connection Error"), error);
        ui->dcLabel->setText("Error");
        ui->dcLabel->setStyleSheet("QLabel { color : red; }");
        qWarning() << "Connection error:" << error;
    });

    connect(audioSink, &BluetoothA2DPSink::stateChanged, this, [](const QString &state) {
        qDebug() << "Sink state:" << state;
    });

    //create and connect the bluetooth discovery agent
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &PhoneAudioLink::appendDevice);

    //error catching
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, [this](QBluetoothDeviceDiscoveryAgent::Error e){
        qDebug()<<e;
        if(e == QBluetoothDeviceDiscoveryAgent::NoError) return;
        else if(e == 2)
            QMessageBox::critical(this, tr("Bluetooth Error"), tr("PoweredOffError: Make sure that bluetooth is turned on."));
        else
            QMessageBox::critical(this, tr("Bluetooth Error"), tr("Unknown: Open a bug report with this info: %1").arg(e));
    });

    startDiscovery(); //automatically look for devices

    //load initialization data from "init.json" if it exists
    loadInitData();

    if(maximizeBluetoothCompatability)
        ui->info->setToolTip("Showing all devices for compatability's sake.\nNot all of these devices are guaranteed to be supported.");
    else
        ui->info->setToolTip("Filtering for only phone devices.\nUse Advanced->Maximize Bluetooth compatability to show more devices.");

    //create a system tray icon
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/icon.ico"), this);

    //create the tray context menu
    trayMenu = new QMenu(this);
    QTimer::singleShot(2000, this, &PhoneAudioLink::updateTrayContext);

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

    // Connect UI buttons to A2DP sink
    connect(ui->playPause, &QPushButton::pressed, this, &PhoneAudioLink::playPause);
    connect(ui->forward, &QPushButton::pressed, this, [this]() {
        if (audioSink) audioSink->sendNext();
    });
    connect(ui->back, &QPushButton::pressed, this, [this]() {
        if (audioSink) audioSink->sendPrevious();
    });
    connect(ui->refresh, &QPushButton::pressed, this, &PhoneAudioLink::startDiscovery);
    connect(ui->connect, &QPushButton::pressed, this, &PhoneAudioLink::connectSelectedDevice);
    connect(ui->disconnect, &QPushButton::pressed, this, &PhoneAudioLink::disconnect);
    connect(ui->deviceComboBox, &QComboBox::currentIndexChanged, this,
            &PhoneAudioLink::deviceComboChanged);

    // Setup Menu Actions
    ui->compatAction->setChecked(maximizeBluetoothCompatability);
    ui->startMinimizedAction->setChecked(startMinimized);

    connect(ui->compatAction, &QAction::triggered, this, [this](bool checked){
        maximizeBluetoothCompatability=checked;

        //change the info tooltip
        if(maximizeBluetoothCompatability)
            ui->info->setToolTip("Showing all devices for compatability's sake.\nNot all of these devices are guaranteed to support A2DP.");
        else
            ui->info->setToolTip("Filtering for only phone devices.\nUse Advanced->Maximize Bluetooth compatability to show more devices.");

        //refresh devices
        startDiscovery();
    });

    connect(ui->menuConnectOnLaunch, &QMenu::hovered, this, [this](){
        static int c;
        if(c == 50){
            this->updateAutoConnectMenu();
            this->updateTrayContext();
            c = 0;
        }
        c++;
    });

    QTimer::singleShot(2000, this, &PhoneAudioLink::updateAutoConnectMenu);

    connect(ui->startMinimizedAction, &QAction::triggered, this, [this](bool checked){
        this->startMinimized=checked;
        this->updateTrayContext();
        this->updateAutoConnectMenu();
    });

    startupHelp = new StartupHelp(this);

    connect(ui->startOnLoginAction, &QAction::triggered, this, [this](){
        this->startupHelp->exec();
    });

    connect(ui->versionAction, &QAction::triggered, this, [this](){
        QMessageBox::information(this, tr("Program Version"), tr("Version %1\t").arg(GLOBAL_PROGRAM_VERSION));
    });

    connect(ui->info, &QPushButton::clicked, this, [this](){
        QToolTip::showText(this->mapToGlobal(ui->info->pos()), ui->info->toolTip(), this, {}, 10000);
    });

    connect(ui->debug, &QAction::triggered, this, [this](){
        qDebug()<<"name: "<<ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().name();
        QBluetoothLocalDevice localDevice;
        qDebug()<<"state: "<<localDevice.pairingStatus(ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().address());
        qDebug()<<"address:"<<ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().address();

        this->updateTrayContext();
        this->updateAutoConnectMenu();
    });

#ifdef RELEASE_BUILD
    ui->debug->setVisible(false);
#endif

    // Initial button states
    ui->connect->setEnabled(true);
    ui->disconnect->setEnabled(false);

    // Auto-connect if enabled and device was saved
    if (connectAutomatically) {
        QTimer::singleShot(2000, this, [this](){
            QString name = this->findDeviceName(savedDeviceAddress);
            int index = this->ui->deviceComboBox->findText(name);
            if(this->ui->deviceComboBox->count() > 0 && index != -1){
                this->ui->deviceComboBox->setCurrentIndex(index);
                this->connectSelectedDevice();
                this->updateTrayContext();
                this->updateAutoConnectMenu();
            }
        });
    }
}

//destructor
PhoneAudioLink::~PhoneAudioLink() {
    // Stop device watchers
    if (audioSink) {
        audioSink->stopDeviceDiscovery();
    }
    if(discoveryAgent){
        discoveryAgent->stop();//stop bluetooth discovery
        discoveryAgent->deleteLater();
        discoveryAgent = nullptr;
    }
    if(ui->dcLabel->text() == "Connected")
        ui->disconnect->click();
    delete ui;
}

//public function that returns if the program should start in a minimized state
bool PhoneAudioLink::getStartMinimized(){
    return this->startMinimized;
}

//detect window minimize event
void PhoneAudioLink::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            saveInitData();
            QTimer::singleShot(0, this, &PhoneAudioLink::hide); // hide after minimize animation
        }
    }
    QMainWindow::changeEvent(event); //call the super method
}

void PhoneAudioLink::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    windowShown = true;
    updateTrayContext();
}

void PhoneAudioLink::hideEvent(QHideEvent *event) {
    QMainWindow::hideEvent(event);
    windowShown = false;
    updateTrayContext();
}

//show the window from tray
void PhoneAudioLink::showFromTray() {
    // show();
    // setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    // raise();
    showNormal();
    raise();
    activateWindow();
    // activateWindow();

}

void PhoneAudioLink::updateTrayContext(){
    trayMenu->clear();
    for(auto* i:std::as_const(trayDeviceActions))
        delete i;
    trayDeviceActions.clear();
    for(auto * i:std::as_const(trayDeviceStartupActions))
        delete i;
    trayDeviceStartupActions.clear();

    QMenu *devicesMenu = new QMenu("Connect");
    for (int i = 0; i < ui->deviceComboBox->count(); i++) {
        trayDeviceActions.append(new QAction(ui->deviceComboBox->itemText(i)));
        trayDeviceActions.last()->setCheckable(true);
        if(ui->dcLabel->text() == "Connected" && trayDeviceActions.last()->text() == connectedDevice) trayDeviceActions.last()->setChecked(true);
        else trayDeviceActions.last()->setChecked(false);
        connect(trayDeviceActions.last(), &QAction::triggered, this, [this, i](){
            ui->deviceComboBox->setCurrentIndex(i);
            connectSelectedDevice();
        });
        devicesMenu->addAction(trayDeviceActions.last());
    }

    QAction *disconnectAction = new QAction("Disconnect", this);
    disconnectAction->setCheckable(false);
    if(ui->dcLabel->text() == "Disconnected!") disconnectAction->setDisabled(true);
    else disconnectAction->setDisabled(false);

    QMenu *settingsMenu = new QMenu("Settings");

    QAction *startMinimized = new QAction("Start Minimized", this);
    QMenu *autoConnect = new QMenu("Connect on Launch", this);
    for (int i = 0; i < ui->deviceComboBox->count(); i++) {
        trayDeviceStartupActions.append(new QAction(ui->deviceComboBox->itemText(i)));
        trayDeviceStartupActions.last()->setCheckable(true);
        if(connectAutomatically && [this](){
                for (const QBluetoothDeviceInfo &info : std::as_const(this->discoveredDevices))
                    if (info.address() == this->savedDeviceAddress && info.name() == this->trayDeviceStartupActions.last()->text())
                        return true;
                return false;
            }()) trayDeviceStartupActions.last()->setChecked(true);
        else trayDeviceStartupActions.last()->setChecked(false);
        connect(trayDeviceStartupActions.last(), &QAction::triggered, this, [this, i](){
            if(!this->connectAutomatically){
                connectAutomatically = true;
                this->savedDeviceAddress = ui->deviceComboBox->itemData(i).value<QBluetoothDeviceInfo>().address();
            }
            else connectAutomatically = false;
            this->updateAutoConnectMenu();
        });
        autoConnect->addAction(trayDeviceStartupActions.last());
    }

    QAction *autoStart = new QAction("Start on Login", this);

    startMinimized->setCheckable(true);
    // autoConnect->setCheckable(true);
    autoStart->setCheckable(false);

    startMinimized->setChecked(this->startMinimized);
    // autoConnect->setChecked(connectAutomatically);

    settingsMenu->addAction(startMinimized);
    settingsMenu->addMenu(autoConnect);
    settingsMenu->addAction(autoStart);

    QAction *restoreAction = new QAction("Show", this);
    if(windowShown) restoreAction->setDisabled(true);
    else restoreAction->setDisabled(false);

    QAction *quitAction = new QAction("Exit", this);

    quitAction->setCheckable(false);

    //connect the tray context menu buttons to their respective actions
    connect(startMinimized, &QAction::triggered, ui->startMinimizedAction, &QAction::trigger);
    // connect(autoConnect, &QAction::triggered, ui->connectStartupAction, &QAction::trigger);
    connect(disconnectAction, &QAction::triggered, ui->disconnect, &QPushButton::click);
    connect(autoStart, &QAction::triggered, ui->startOnLoginAction, &QAction::trigger);
    connect(restoreAction, &QAction::triggered, this, &PhoneAudioLink::showFromTray);
    connect(quitAction, &QAction::triggered, this, &PhoneAudioLink::exitApp);

    //add the actions
    trayMenu->addMenu(devicesMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(disconnectAction);
    trayMenu->addSeparator();
    // trayMenu->addAction(startMinimized);
    // trayMenu->addAction(autoConnect);
    // trayMenu->addAction(autoStart);
    trayMenu->addMenu(settingsMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(restoreAction);
    trayMenu->addAction(quitAction);

    //configure and show the tray icon
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void PhoneAudioLink::updateAutoConnectMenu() {
    ui->menuConnectOnLaunch->clear();
    for (int i = 0; i < ui->deviceComboBox->count(); i++) {
        autoConnectMenuActions.append(new QAction(ui->deviceComboBox->itemText(i)));
        autoConnectMenuActions.last()->setCheckable(true);
        if(connectAutomatically && [this](){
                for (const QBluetoothDeviceInfo &info : std::as_const(this->discoveredDevices))
                    if (info.address() == this->savedDeviceAddress && info.name() == this->autoConnectMenuActions.last()->text())
                        return true;
                return false;
            }()) autoConnectMenuActions.last()->setChecked(true);
        else autoConnectMenuActions.last()->setChecked(false);
        connect(autoConnectMenuActions.last(), &QAction::triggered, this, [this, i](){
            if(!this->connectAutomatically){
                connectAutomatically = true;
                this->savedDeviceAddress = ui->deviceComboBox->itemData(i).value<QBluetoothDeviceInfo>().address();
            }
            else connectAutomatically = false;
            this->updateTrayContext();
        });
        ui->menuConnectOnLaunch->addAction(autoConnectMenuActions.last());
    }
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
//TODO: change this to toggled so I know if we're paused or playing


    // Send play/pause command via A2DP sink
    if (audioSink)
        audioSink->sendPlayPause();
}

void PhoneAudioLink::startDiscovery() {

    // Reset current
    ui->deviceComboBox->clear();
    discoveredDevices.clear();
    deviceIdMap.clear();

    // Start Qt Bluetooth discovery (for display/pairing info)
    discoveryAgent->stop();
    discoveryAgent->start();

    // Also start Windows device discovery for A2DP connection
    if (audioSink) {
        audioSink->startDeviceDiscovery();
    }

    // Update context menu after giving time for bluetooth discovery
    QTimer::singleShot(3000, this, &PhoneAudioLink::updateTrayContext);
}

void PhoneAudioLink::onA2DPDeviceDiscovered(const QString &deviceId, const QString &deviceName) {
    qDebug() << "A2DP device discovered:" << deviceName << "with ID:" << deviceId;

    // Store the Windows device ID
    deviceIdMap[deviceName] = deviceId;
}

void PhoneAudioLink::onA2DPDiscoveryCompleted() {
    qDebug() << "A2DP discovery completed. Found" << deviceIdMap.size() << "devices!";
}

void PhoneAudioLink::appendDevice(const QBluetoothDeviceInfo &device) {
    //filter devices
    if(device.name().startsWith("Bluetooth") && device.name().contains(":")) {
        return;
    }
    // qDebug()<<"discovered device";
    // qDebug()<<"\tName: "               <<device.name();
    // qDebug()<<"\tMajor, Minor Device Classes: " <<device.majorDeviceClass()<<device.minorDeviceClass();
    // qDebug()<<"\tDevice address: "<<device.address();

    QString tag = "";
    bool isPhone = device.majorDeviceClass() == QBluetoothDeviceInfo::PhoneDevice;
    bool isAv = device.majorDeviceClass() == QBluetoothDeviceInfo::AudioVideoDevice;

    if(maximizeBluetoothCompatability){
        //set tag based off of device type
        if(isPhone) tag=" [Phone Device]";
        else if(isAv) tag=" [AV Device]";
        else tag=" [UNKNOWN]";

        //add the device
        ui->deviceComboBox->addItem(device.name()+tag, QVariant::fromValue(device));
        discoveredDevices.append(device);

        //if it matches the saved device, set that to the current index
        if(device.address() == savedDeviceAddress)
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findData(QVariant::fromValue(device)));
        return;
    }

    if(isPhone){
        //add the device
        ui->deviceComboBox->addItem(device.name(), QVariant::fromValue(device));
        discoveredDevices.append(device);

        //if it matches the saved device, set that to the current index
        if(device.address() == savedDeviceAddress)
            ui->deviceComboBox->setCurrentIndex(ui->deviceComboBox->findData(QVariant::fromValue(device)));
        return;
    }
}

//connect to the device in the combo box
void PhoneAudioLink::connectSelectedDevice() {

    QString selectedText = ui->deviceComboBox->currentText();

    // Remove any tags like [A2DP], [Phone Device], etc.
    QString deviceName = selectedText;
    deviceName.remove(QRegularExpression("\\s*\\[.*\\]\\s*"));

    qDebug() << "Attempting to connect to:" << deviceName;

    if (audioSink && deviceIdMap.contains(deviceName)) {
        // Use the Windows device ID we got from DeviceWatcher
        QString windowsDeviceId = deviceIdMap[deviceName];

        qDebug() << "Using Windows device ID:" << windowsDeviceId;

        ui->dcLabel->setText("Connecting...");
        ui->dcLabel->setStyleSheet("QLabel { color : orange; }");

        // Enable the A2DP sink for this device
        audioSink->enableSink(windowsDeviceId);

        // wait a bit, then update the connected device
        QTimer::singleShot(3000, this, [this, deviceName](){
            if(ui->dcLabel->text() == "Connected")
                this->connectedDevice = deviceName;
            else //wait another 3 seconds if not connected yet
                QTimer::singleShot(3000, this, [this, deviceName](){
                    if(ui->dcLabel->text() == "Connected") //if still not connected, then it will fail out before this happens
                        this->connectedDevice = deviceName;
                    this->updateTrayContext();
                });
            this->updateTrayContext();
        });
    } else {
        qWarning() << "Device not found in A2DP device map:" << deviceName;
        QMessageBox::warning(this, tr("Connection Error"),
                             tr("Device not found. Please make sure the device supports A2DP and try refreshing the device list."));
    }

    updateTrayContext();
}

void PhoneAudioLink::disconnect() {
    if (audioSink) {
        audioSink->releaseConnection();
    }

    updateTrayContext();
}

//triggers when the index of the device combo box is changed
void PhoneAudioLink::deviceComboChanged(int i){
    Q_UNUSED(i);
    //qDebug()<<"changed index: "<<i;
    updateAutoConnectMenu();
    updateTrayContext();
}

//save initialization data
void PhoneAudioLink::saveInitData() {
    //initialize the file object
    QString fileName = QCoreApplication::applicationDirPath()+"/init.json";
    QJsonObject config;

    config["maximizeBluetoothCompatability"] = maximizeBluetoothCompatability;
    config["connectAutomatically"] = connectAutomatically;
    config["startMinimized"] = startMinimized;
    config["device"] = savedDeviceAddress.toString();//ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().address().toString();

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

    updateTrayContext();

}

//load initialization data
void PhoneAudioLink::loadInitData(){
    //initialize file object
    QString initFilePath = QCoreApplication::applicationDirPath()+"/init.json";
    QFile initFile(initFilePath);
    bool err = false;

    //check if init.json exists
    if(initFile.exists()){
        //read the file
        if(!initFile.open(QIODevice::ReadOnly)){
            QMessageBox::critical(this, tr("Error: FileReadError"), tr("Failed to open configuration file \'init.config\'"));
            err = true;
        }
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
            err = true;
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

//for debugging purposes
QString PhoneAudioLink::stringifyUuids(QList<QBluetoothUuid> l){
    QString result = "";
    for(QBluetoothUuid i:l){
        result+=i.toString()+", ";
    }
    return result;
}

QString PhoneAudioLink::findDeviceName(const QBluetoothAddress& address){
    for (const QBluetoothDeviceInfo &info : discoveredDevices) {
        if (info.address() == address) {
            return info.name();
        }
    }

    // Not found
    return QString();
}
