TARGET = systemd-journal-daemon
target.path = /usr/bin

QT += dbus
CONFIG += link_pkgconfig
PKGCONFIG += libsystemd


SOURCES += src/daemon.cpp \
    src/adaptor.cpp \
    src/journal.cpp

dbus.files = dbus/org.coderus.systemd.journal.service
dbus.path = /usr/share/dbus-1/services

systemd.files = systemd/systemd-journal-daemon.service
systemd.path = /usr/lib/systemd/user

journalconf.files = systemd/90_systemd-journal-daemon.conf
journalconf.path = /etc/systemd/journald.conf.d

INSTALLS += target dbus systemd journalconf

HEADERS += \
    src/adaptor.h \
    src/journal.h

