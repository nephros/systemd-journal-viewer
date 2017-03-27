#include "adaptor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>

Adaptor::Adaptor(QObject *parent) : QObject(parent)
{

}

void Adaptor::start()
{
    if (QDBusConnection::sessionBus().registerService("ru.omprussia.systemd.journal")) {
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);

        m_journal = new Journal(0);

        QThread *thread = new QThread(this);
        thread->setObjectName(QString("journalThread-%1").arg(QDateTime::currentMSecsSinceEpoch()));
        m_journal->moveToThread(thread);
        connect(thread, &QThread::started, m_journal, &Journal::init);
        connect(m_journal, &Journal::destroyed, thread, &QThread::quit);
        connect(m_journal, &Journal::quit, this, &Adaptor::quit);

        connect(m_journal, &Journal::dataReceived, this, &Adaptor::dataReceived);

        connect(this, &Adaptor::doAddMatch, m_journal, &Journal::addMatch, Qt::DirectConnection);
        connect(this, &Adaptor::doFlushMatches, m_journal, &Journal::flushMatches, Qt::DirectConnection);
        connect(this, &Adaptor::doSkipTail, m_journal, &Journal::skipTail, Qt::DirectConnection);
        connect(this, &Adaptor::doSeekTimestamp, m_journal, &Journal::seekTimestamp, Qt::DirectConnection);

        thread->start();
    }
    else {
        qApp->quit();
    }
}

void Adaptor::ping()
{

}

void Adaptor::quit()
{
    qApp->quit();
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
