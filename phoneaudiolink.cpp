#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{
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

