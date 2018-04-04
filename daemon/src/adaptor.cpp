#include "adaptor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>

#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

Adaptor::Adaptor(QObject *parent)
    : QObject(parent)
{

}

void Adaptor::start()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(QStringLiteral("/"), this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals)) {
        qCritical() << "Cannot register object";
        qApp->quit();
    }
    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.coderus.systemd.journal"))) {
        qCritical() << "Cannot register service";
        qApp->quit();
    }

    m_journal = new Journal(this);

    connect(m_journal, &Journal::quit, this, &Adaptor::quit);
    connect(m_journal, &Journal::dataReceived, this, &Adaptor::dataReceived);

    connect(this, &Adaptor::doAddMatch, m_journal, &Journal::addMatch, Qt::DirectConnection);
    connect(this, &Adaptor::doFlushMatches, m_journal, &Journal::flushMatches, Qt::DirectConnection);
    connect(this, &Adaptor::doSkipTail, m_journal, &Journal::skipTail, Qt::DirectConnection);
    connect(this, &Adaptor::doSeekTimestamp, m_journal, &Journal::seekTimestamp, Qt::DirectConnection);
}

void Adaptor::ping()
{
    qDebug() << Q_FUNC_INFO;
}

void Adaptor::quit()
{
    qDebug() << Q_FUNC_INFO;
    qApp->quit();
}

void Adaptor::init()
{
    if (!m_journal) {
        qCritical() << "Journal does not exists!";
    }
    m_journal->init();
}

void Adaptor::addMatch(const QString &match)
{
    emit doAddMatch(match);
}

void Adaptor::flushMatches()
{
    emit doFlushMatches();
}

void Adaptor::skipTail(int size)
{
    emit doSkipTail(size);
}

void Adaptor::seekTimestamp(quint64 timestamp)
{
    emit doSeekTimestamp(timestamp);
}

void Adaptor::saveJournal()
{
    qDebug() << Q_FUNC_INFO;

    const QStringList locations = { QStringLiteral("/var/log/journal"), QStringLiteral("/run/log/journal") };
    for (const QString &location : locations) {
        QDir journalDir(location);
        if (!journalDir.exists()) {
            qWarning() << "Directory does not exist" << journalDir.absolutePath();
            continue;
        }

        const struct passwd *userPasswd = getpwuid(100000);

        QProcess tar;
        QString filename = QStringLiteral("/home/%1/Documents/journal-%2.tar")
            .arg(QString(userPasswd->pw_name))
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_hh-mm-ss-zzz")));
        tar.start(QStringLiteral("/bin/tar"), { QStringLiteral("cvf"), filename, location });
        tar.waitForFinished(-1);
        if (!QFileInfo::exists(filename)) {
            qWarning() << "tar failed!" << tar.readAll();
            return;
        }

        const struct group *userGroup = getgrgid(100000);

        chown(filename.toLatin1().constData(), userPasswd->pw_uid, userGroup->gr_gid);
        chmod(filename.toLatin1().constData(), 0644);
    }
}
