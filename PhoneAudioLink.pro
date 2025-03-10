QT       += core gui bluetooth multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = icon.ico

# INCLUDEPATH += "C:/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0" \
#                "C:/gstreamer/1.0/msvc_x86_64/include/glib-2.0" \
#                "C:/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include"

# LIBS += -LC:/gstreamer/1.0/msvc_x86_64/lib -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0

SOURCES += \
    a2dpstreamer.cpp \
    animatedbutton.cpp \
    main.cpp \
    phoneaudiolink.cpp \
    startuphelp.cpp

HEADERS += \
    a2dpstreamer.h \
    animatedbutton.h \
    phoneaudiolink.h \
    startuphelp.h

FORMS += \
    phoneaudiolink.ui

TRANSLATIONS += \
    PhoneAudioLink_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    README.md
