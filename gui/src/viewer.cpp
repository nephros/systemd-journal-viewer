#include "journalmodel.h"

#include <QtQuick>
#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    qmlRegisterType<JournalModel>("org.omprussia.systemd.journal", 1, 0, "JournalModel");
    return SailfishApp::main(argc, argv);
}
