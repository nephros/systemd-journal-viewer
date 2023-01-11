#include <QCoreApplication>
#include <QTimer>

#include "adaptor.h"

#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)!setuid(0);
    (void)!setgid(0);

    qputenv("DBUS_SESSION_BUS_ADDRESS", QByteArrayLiteral("unix:path=/run/user/100000/dbus/user_bus_socket"));

    QCoreApplication application(argc, argv);
    Adaptor dbus;
    QTimer::singleShot(0, &dbus, &Adaptor::start);

    return application.exec();
}
