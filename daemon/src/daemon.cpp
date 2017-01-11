#include <QCoreApplication>
#include <QTimer>

#include "adaptor.h"

#include <unistd.h>

int main(int argc, char *argv[])
{
    setuid(0);
    setgid(0);

    qputenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/run/user/100000/dbus/user_bus_socket");

    QScopedPointer<QCoreApplication> application(new QCoreApplication(argc, argv));
    QScopedPointer<Adaptor> dbus(new Adaptor());
    QTimer::singleShot(1, dbus.data(), SLOT(start()));

    return application->exec();
}
