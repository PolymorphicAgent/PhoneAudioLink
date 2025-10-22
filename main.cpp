#include "phoneaudiolink.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <iomanip>
#include <sstream>
#include <iostream>

QString version(){
    std::stringstream tmp;
    tmp<<std::fixed<<std::setprecision(GLOBAL_MINOR_PROGRAM_VERSION_SIZE);
    tmp<<GLOBAL_PROGRAM_VERSION;
    return QString::fromStdString(tmp.str());
}

int main(int argc, char *argv[])
{
    qputenv("QT_LOGGING_RULES", "qt.qpa.fonts=false");

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "PhoneAudioLink_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    PhoneAudioLink w;
    w.setWindowTitle("Phone Audio Link v"+version());
    if(!w.getStartMinimized())w.show();
    return a.exec();
}
