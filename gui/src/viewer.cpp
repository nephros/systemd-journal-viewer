#include "filelog.h"

#include <QtQuick>
#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    qmlRegisterType<FileLog>("org.omprussia.systemd.journal", 1, 0, "FileLog");
    return SailfishApp::main(argc, argv);
}
