#include "journalmodel.h"

#include <QtQuick>
#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    qmlRegisterType<JournalModel>("org.coderus.systemd.journal", 1, 0, "JournalModel");
    return SailfishApp::main(argc, argv);
}
