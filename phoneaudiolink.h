#ifndef PHONEAUDIOLINK_H
#define PHONEAUDIOLINK_H

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

private:
    Ui::PhoneAudioLink *ui;
};
#endif // PHONEAUDIOLINK_H
