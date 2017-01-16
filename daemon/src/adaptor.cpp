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
        QObject::connect(thread, SIGNAL(started()), m_journal, SLOT(init()));
        QObject::connect(m_journal, SIGNAL(destroyed()), thread, SLOT(quit()));
        QObject::connect(m_journal, SIGNAL(quit()), this, SLOT(quit()));

        QObject::connect(m_journal, SIGNAL(dataReceived(QVariantMap)), this, SIGNAL(dataReceived(QVariantMap)));

        QObject::connect(this, SIGNAL(doAddMatch(QString)), m_journal, SLOT(addMatch(QString)), Qt::DirectConnection);
        QObject::connect(this, SIGNAL(doFlushMatches()), m_journal, SLOT(flushMatches()), Qt::DirectConnection);
        QObject::connect(this, SIGNAL(doSkipTail(int)), m_journal, SLOT(skipTail(int)), Qt::DirectConnection);
        QObject::connect(this, SIGNAL(doSeekTimestamp(quint64)), m_journal, SLOT(seekTimestamp(quint64)), Qt::DirectConnection);

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
