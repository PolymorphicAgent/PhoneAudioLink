#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

#include <QRegularExpression>

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

        ui->dcLabel->setText("Disconnected");
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
    ui->connectStartupAction->setChecked(connectAutomatically);
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

    connect(ui->info, &QPushButton::clicked, this, [this](){
        QToolTip::showText(this->mapToGlobal(ui->info->pos()), ui->info->toolTip(), this, {}, 10000);
    });

    connect(ui->debug, &QAction::triggered, this, [this](){
        qDebug()<<"name: "<<ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().name();
        QBluetoothLocalDevice localDevice;
        qDebug()<<"state: "<<localDevice.pairingStatus(ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().address());
        qDebug()<<"Service uuids:"<<ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>().serviceUuids();
    });

    // Initial button states
    ui->connect->setEnabled(true);
    ui->disconnect->setEnabled(false);

    // Auto-connect if enabled and device was saved
    if (connectAutomatically && ui->deviceComboBox->count() > 0) {
        QTimer::singleShot(2000, this, &PhoneAudioLink::connectSelectedDevice);
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
//TODO: change this to toggled so I know if we're paused or playing
    // //delegate the play/pause command to A2DPStreamer.
    // if (a2dpStreamer)
    //     a2dpStreamer->playPause();

    // // Delegate the play/pause command to the audio controller
    // if (audioController)
    //     audioController->playPause();

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
}

void PhoneAudioLink::onA2DPDeviceDiscovered(const QString &deviceId, const QString &deviceName) {
    qDebug() << "A2DP device discovered:" << deviceName << "with ID:" << deviceId;

    // Store the Windows device ID
    deviceIdMap[deviceName] = deviceId;

    // // Check if this device is already in the combo box
    // bool found = false;
    // for (int i = 0; i < ui->deviceComboBox->count(); ++i) {
    //     if (ui->deviceComboBox->itemText(i).contains(deviceName)) {
    //         found = true;
    //         break;
    //     }
    // }

    // // If not found, add it
    // if (!found) {
    //     ui->deviceComboBox->addItem(deviceName + " [A2DP]");
    // }
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
    } else {
        qWarning() << "Device not found in A2DP device map:" << deviceName;
        QMessageBox::warning(this, tr("Connection Error"),
                             tr("Device not found. Please make sure the device supports A2DP and try refreshing the device list."));
    }

    // // get the device info from the ComboBox
    // auto device = ui->deviceComboBox->currentData().value<QBluetoothDeviceInfo>();

    // if (audioSink) {
    //     // Convert Bluetooth address to device ID format
    //     // Windows uses a specific format: BluetoothLE#BluetoothLE{MAC}-{SHORT_ID}
    //     // For A2DP, we need the full device instance ID
    //     // Qt gives us the MAC address, we need to construct the device ID

    //     QString macAddress = device.address().toString().remove(':').toUpper();

    //     // Windows AudioPlaybackConnection expects a device ID in this format:
    //     // \\?\BTHENUM#{UUID}_{MAC}
    //     // However, we can also use the simpler BT address format that AudioPlaybackConnection accepts
    //     QString deviceId = macAddress;

    //     qDebug() << "Connecting to device:" << device.name();
    //     qDebug() << "Device ID:" << deviceId;

    //     ui->dcLabel->setText("Connecting...");
    //     ui->dcLabel->setStyleSheet("QLabel { color : orange; }");

    //     // Enable the A2DP sink for this device
    //     audioSink->enableSink(deviceId);
    // }
}

void PhoneAudioLink::disconnect() {
    // if (a2dpStreamer) {
    //     a2dpStreamer->disconnectDevice();
    // }

    // if (audioController) {
    //     audioController->disconnectDevice();
    // }

    if (audioSink) {
        audioSink->releaseConnection();
    }
}

//triggers when the index of the device combo box is changed
void PhoneAudioLink::deviceComboChanged(int i){
    //TODO: implement greying out of buttons, changing color/text of label, etc.
    Q_UNUSED(i);
    //qDebug()<<"changed index: "<<i;
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

//for debugging purposes
QString PhoneAudioLink::stringifyUuids(QList<QBluetoothUuid> l){
    QString result = "";
    for(QBluetoothUuid i:l){
        result+=i.toString()+", ";
    }
    return result;
}
