#include "journal.h"

#include <QVariantMap>
#include <QDebug>
#include <stdio.h>

#define JOURNAL_FOREACH_DATA_RETVAL(j, data, l, retval) \
    for (sd_journal_restart_data(j); ((retval) = sd_journal_enumerate_data((j), &(data), &(l))) > 0; )

Journal::Journal(QObject *parent)
    : QObject(parent)
{

}

void Journal::addMatch(const QString &match)
{
    if (sdj) {
        sd_journal_add_match(sdj, match.toUtf8().constData(), 0);

        process();
    }
}

void Journal::flushMatches()
{
    if (sdj) {
        sd_journal_flush_matches(sdj);

        process();
    }
}

void Journal::skipTail(int size)
{
    if (sdj) {
        if (sd_journal_seek_tail(sdj) == 0) {
            if (size <= 0) {
                sd_journal_previous(sdj);
            } else {
                sd_journal_previous_skip(sdj, (uint64_t)size);
            }

            process();
        }
    }
}

void Journal::seekTimestamp(quint64 timestamp)
{
    if (sdj) {
        sd_journal_seek_realtime_usec(sdj, (uint64_t)timestamp * 1000);

        process();
    }
}

void Journal::wait()
{
    JournalWaiter *jw = new JournalWaiter(sdj);
    QThread *thread = new QThread(this);
    thread->setObjectName(QStringLiteral("journalWaiter-%1").arg(QDateTime::currentMSecsSinceEpoch()));
    jw->moveToThread(thread);
    connect(thread, &QThread::started, jw, &JournalWaiter::start);
    connect(jw, &JournalWaiter::destroyed, thread, &QThread::quit);
    connect(jw, &JournalWaiter::pollAgain, this, &Journal::wait);
    connect(jw, &JournalWaiter::pollFailed, this, &Journal::quit);
    connect(jw, &JournalWaiter::canProcess, this, &Journal::process);
    thread->start();
}

void Journal::init()
{
    if (sd_journal_open(&sdj, SD_JOURNAL_LOCAL_ONLY) < 0) {
        emit quit();
        return;
    }

    if (sd_journal_get_fd(sdj) < 0) {
        perror("Cannot get journal descriptor");
        emit quit();
        return;
    }

    skipTail(50);

    process();
}

void Journal::process()
{
    int next_ret = sd_journal_next(sdj);

    QVariantList dataList;

    while (next_ret > 0) {
        const void *data;
        size_t length;
        int ret;

        QVariantMap jsonData;

        JOURNAL_FOREACH_DATA_RETVAL(sdj, data, length, ret) {
            QString fieldLine = QString::fromUtf8((const char*)data, length);
            int fieldIndex = fieldLine.indexOf(QChar('='));
            QString fieldName = fieldLine.left(fieldIndex);
            QString fieldData = fieldLine.mid(fieldIndex + 1);
            jsonData[fieldName] = fieldData;
        }

        uint64_t realtime;
        if (sd_journal_get_realtime_usec(sdj, &realtime) < 0) {
            emit quit();
            break;
        }

        jsonData["__TIMESTAMP"] = realtime / 1000;
        dataList.append(jsonData);

        if (dataList.length() == 1000) {
            emit dataReceived(dataList);
            dataList.clear();

            QCoreApplication::processEvents();
        }
        next_ret = sd_journal_next(sdj);
    }

    if (dataList.length() > 0) {
        emit dataReceived(dataList);
    }

    if (next_ret == 0) {
        wait();
    } else {
        sd_journal_close(sdj);
        emit quit();
    }
}
