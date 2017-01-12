#include "journal.h"

#include <QVariantMap>
#include <stdio.h>

#define JOURNAL_FOREACH_DATA_RETVAL(j, data, l, retval)                     \
        for (sd_journal_restart_data(j); ((retval) = sd_journal_enumerate_data((j), &(data), &(l))) > 0; )

int get_tail(sd_journal *j)
{
    if (sd_journal_seek_tail(j) < 0)
        return -1;
    return sd_journal_previous(j);
}

Journal::Journal(QObject *parent) : QObject(parent)
{

}

void Journal::addMatch(const QString &match)
{
    if (sdj) {
        sd_journal_add_match(sdj, match.toLatin1().constData(), 0);
    }
}

void Journal::flushMatches()
{
    if (sdj) {
        sd_journal_flush_matches(sdj);
    }
}

void Journal::skipTail(int size)
{
    if (sdj) {
        if (sd_journal_seek_tail(sdj) == 0) {
            sd_journal_previous_skip(sdj, (uint64_t)size);
        }
    }
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

    if (get_tail(sdj) < 0) {
        perror("Cannot obtain journal tail");
        emit quit();
        return;
    }

    for (;;) {
        int next_ret;
        next_ret = sd_journal_next(sdj);
        if (next_ret > 0) {
            const void *data;
            size_t length;
            int ret;

            QVariantMap jsonData;

            JOURNAL_FOREACH_DATA_RETVAL(sdj, data, length, ret) {
                QString fieldLine = QString::fromUtf8((const char*)data, length);
                int fieldIndex = fieldLine.indexOf(QStringLiteral("="));
                QString fieldName = fieldLine.left(fieldIndex);
                QString fieldData = fieldLine.mid(fieldIndex + 1);
                jsonData[fieldName] = fieldData;
            }

            uint64_t realtime;
            if (sd_journal_get_realtime_usec(sdj, &realtime) == 0) {
                jsonData["__TIMESTAMP"] = realtime;
            }

            emit dataReceived(jsonData);
        } else if (next_ret == 0) {
            if (sd_journal_wait(sdj, (uint64_t)-1) < 0) {
                emit quit();
                return;
            }
        } else if (next_ret < 0) {
            emit quit();
            return;
        }
    }

    sd_journal_close(sdj);

    emit quit();
    return;
}
