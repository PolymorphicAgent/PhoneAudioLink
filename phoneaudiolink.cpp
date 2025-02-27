#include "phoneaudiolink.h"
#include "ui_phoneaudiolink.h"

PhoneAudioLink::PhoneAudioLink(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneAudioLink)
{
    ui->setupUi(this);
}

PhoneAudioLink::~PhoneAudioLink()
{
    delete ui;
}
