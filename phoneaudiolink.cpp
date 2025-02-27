#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{
    ui->setupUi(this);
    ui->dcLabel->setStyleSheet("QLabel { color : red; }");
    connect(ui->playPause, SIGNAL(pressed()), this, SLOT(playPause()));
}

PhoneAudioLink::~PhoneAudioLink()
{
    delete ui;
}

void PhoneAudioLink::playPause()
{
    ui->playPause->toggleState();
}

