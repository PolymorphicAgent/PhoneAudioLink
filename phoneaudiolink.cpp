#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{
    ui->setupUi(this);
    ui->label_3->setStyleSheet("QLabel { color : red; }");
}

PhoneAudioLink::~PhoneAudioLink()
{
    delete ui;
}
