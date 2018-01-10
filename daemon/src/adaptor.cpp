#include "adaptor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>

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
    if (QDBusConnection::sessionBus().registerService("ru.omprussia.systemd.journal")) {
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);

        m_journal = new Journal(this);
        m_journal->init();

        connect(m_journal, &Journal::quit, this, &Adaptor::quit);
        connect(m_journal, &Journal::dataReceived, this, &Adaptor::dataReceived);

        connect(this, &Adaptor::doAddMatch, m_journal, &Journal::addMatch, Qt::DirectConnection);
        connect(this, &Adaptor::doFlushMatches, m_journal, &Journal::flushMatches, Qt::DirectConnection);
        connect(this, &Adaptor::doSkipTail, m_journal, &Journal::skipTail, Qt::DirectConnection);
        connect(this, &Adaptor::doSeekTimestamp, m_journal, &Journal::seekTimestamp, Qt::DirectConnection);
    }
    else {
        qApp->quit();
    }
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

void Adaptor::addMatch(const QString &match)
{
    qDebug() << Q_FUNC_INFO << match;
    emit doAddMatch(match);
}

void Adaptor::flushMatches()
{
    qDebug() << Q_FUNC_INFO;
    emit doFlushMatches();
}

void Adaptor::skipTail(int size)
{
    qDebug() << Q_FUNC_INFO << size;
    emit doSkipTail(size);
}

void Adaptor::seekTimestamp(quint64 timestamp)
{
    qDebug() << Q_FUNC_INFO << timestamp;
    emit doSeekTimestamp(timestamp);
}

void Adaptor::saveJournal(const QString &filename)
{
    qDebug() << Q_FUNC_INFO << filename;
    QDir journalDir(QStringLiteral("/var/log/journal"));
    if (!journalDir.exists()) {
        qWarning() << "Directory does not exist" << journalDir.absolutePath();
        return;
    }
    qDebug() << journalDir.absolutePath();
    QStringList folders = journalDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (folders.isEmpty()) {
        qWarning() << "Directory does not contain journal subdir" << journalDir.absolutePath();
        return;
    }
    qDebug() << folders.first();
    journalDir.cd(folders.first());
    qDebug() << journalDir.absolutePath();
    QString journalFileName = journalDir.absoluteFilePath(QStringLiteral("system.journal"));
    if (!QFileInfo::exists(journalFileName)) {
        qWarning() << "Directory does not contain system journal" << journalFileName;
        return;
    }

    if (!QFile::copy(journalFileName, filename)) {
        qWarning() << "Journal save failed" << filename;
        return;
    }

    const struct group *nemoGroup = getgrnam("nemo");
    const struct passwd *nemoPasswd = getpwnam("nemo");

    chown(filename.toLatin1().constData(), nemoPasswd->pw_uid, nemoGroup->gr_gid);
    chmod(filename.toLatin1().constData(), 0644);
}
