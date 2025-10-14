QT       += core gui bluetooth multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = icon.ico

# Windows-specific libraries
win32 {
    QMAKE_CXXFLAGS += /await:strict /std:c++20 # Enable coroutines
    LIBS += -lUser32
    LIBS += -lwindowsapp -lruntimeobject  # WinRT libs
    DEFINES += WINVER=0x0A00 _WIN32_WINNT=0x0A00  # Win10+

    # Require Windows 10 2004 minimum
    DEFINES += NTDDI_VERSION=NTDDI_WIN10_CO

    message("Configuring for Windows with A2DP Sink support")
    message("Requires: Windows 10 version 2004 (May 2020) or later")
    message("Requires: MSVC compiler (MinGW not supported)")
}

# Program Version
DEFINES += GLOBAL_PROGRAM_VERSION="1.1"
DEFINES += GLOBAL_MINOR_PROGRAM_VERSION_SIZE=1

CONFIG(debug, debug|release) {
    DEFINES += DEBUG_BUILD
} else {
    DEFINES += RELEASE_BUILD
}


SOURCES += \
    # a2dpstreamer.cpp \
    animatedbutton.cpp \
    bluetootha2dpsink.cpp \
    # bluetoothaudiocontroller.cpp \
    # bluetoothaudiocontrollerstub.cpp \
    # bluetoothaudiocontrollerwin.cpp \
    main.cpp \
    phoneaudiolink.cpp \
    startuphelp.cpp

HEADERS += \
    # a2dpstreamer.h \
    animatedbutton.h \
    bluetootha2dpsink.h \
    # bluetoothaudiocontroller.h \
    # bluetoothaudiocontrollerstub.h \
    # bluetoothaudiocontrollerwin.h \
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

# Verify MSVC compiler on Windows
win32 {
    !contains(QMAKE_COMPILER, msvc) {
        error("This project requires MSVC compiler for WinRT support. MinGW is not supported.")
    }
}
