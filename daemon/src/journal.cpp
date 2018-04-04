#include "journal.h"

#include <QVariantMap>
#include <QDebug>
#include <stdio.h>

Journal::Journal(QObject *parent)
    : QObject(parent)
{

}

void Journal::addMatch(const QString &match)
{
    if (m_sdj) {
        sd_journal_add_match(m_sdj, match.toUtf8().constData(), 0);
    }
}

void Journal::flushMatches()
{
    if (m_sdj) {
        sd_journal_flush_matches(m_sdj);
    }
}

void Journal::skipTail(int size)
{
    if (m_sdj) {
        if (sd_journal_seek_tail(m_sdj) == 0) {
            if (size > 0) {
               sd_journal_previous_skip(m_sdj, (uint64_t)size);
            }
        }
    }
}

void Journal::seekTimestamp(quint64 timestamp)
{
    if (m_sdj) {
        sd_journal_seek_realtime_usec(m_sdj, (uint64_t)timestamp * 1000);
    }
}

void Journal::wait()
{
    if (m_jw) {
        QMetaObject::invokeMethod(m_jw, "start", Qt::QueuedConnection);
        return;
    }

    m_jw = new JournalWaiter(m_sdj);
    QThread *thread = new QThread(this);
    thread->setObjectName(QStringLiteral("journalWaiter-%1").arg(QDateTime::currentMSecsSinceEpoch()));
    m_jw->moveToThread(thread);

    connect(thread, &QThread::started, m_jw, &JournalWaiter::start);
    connect(m_jw, &JournalWaiter::destroyed, [thread](){
        QMetaObject::invokeMethod(thread, "quit", Qt::QueuedConnection);
    });
    connect(m_jw, &JournalWaiter::pollAgain, [this](){
        QMetaObject::invokeMethod(m_jw, "start", Qt::QueuedConnection);
    });
    connect(m_jw, &JournalWaiter::pollFailed, [this](){
        m_jw->deleteLater();
        m_jw = nullptr;
        emit quit();
    });
    connect(m_jw, &JournalWaiter::canProcess, this, &Journal::process);
    thread->start();
}

void Journal::init()
{
    if (sd_journal_open(&m_sdj, SD_JOURNAL_LOCAL_ONLY) < 0) {
        emit quit();
        return;
    }

    if (sd_journal_get_fd(m_sdj) < 0) {
        perror("Cannot get journal descriptor");
        emit quit();
        return;
    }

    skipTail(50);

    wait();
}

void Journal::process()
{
    int next_ret = sd_journal_next(m_sdj);

    QVariantList dataList;

    while (next_ret > 0) {
        const void *data;
        size_t length;

        QVariantMap jsonData;

        SD_JOURNAL_FOREACH_DATA(m_sdj, data, length) {
            QString fieldLine = QString::fromUtf8((const char*)data, length);
            int fieldIndex = fieldLine.indexOf(QChar('='));
            QString fieldName = fieldLine.left(fieldIndex);
            QString fieldData = fieldLine.mid(fieldIndex + 1);
            jsonData[fieldName] = fieldData;
        }

        uint64_t realtime;
        if (sd_journal_get_realtime_usec(m_sdj, &realtime) < 0) {
            emit quit();
            break;
        }

        jsonData["__TIMESTAMP"] = (quint64)realtime / 1000ULL;
        dataList.append(jsonData);

        if (dataList.length() == 100) {
            emit dataReceived(dataList);
            dataList.clear();

            QCoreApplication::processEvents();
        }
        next_ret = sd_journal_next(m_sdj);
    }

    if (dataList.length() > 0) {
        emit dataReceived(dataList);
        dataList.clear();
    }

    if (next_ret == 0) {
        QMetaObject::invokeMethod(m_jw, "start", Qt::QueuedConnection);
    } else {
        sd_journal_close(m_sdj);
        emit quit();
    }
}
