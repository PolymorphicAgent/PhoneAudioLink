#include "phoneaudiolink.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

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
    if(!w.getStartMinimized())w.show();
    return a.exec();
}
