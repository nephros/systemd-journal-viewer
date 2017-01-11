TARGET = systemd-journal-viewer

QT += dbus
CONFIG += sailfishapp

SOURCES += src/viewer.cpp \
    src/filelog.cpp

OTHER_FILES += qml/systemd-journal-viewer.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    systemd-journal-viewer.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

CONFIG += sailfishapp_i18n

TRANSLATIONS += translations/systemd-journal-viewer-de.ts

HEADERS += \
    src/filelog.h
