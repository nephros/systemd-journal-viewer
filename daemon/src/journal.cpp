#include "journal.h"

#include <QVariantMap>
#include <stdio.h>

#define JOURNAL_FOREACH_DATA_RETVAL(j, data, l, retval) \
    for (sd_journal_restart_data(j); ((retval) = sd_journal_enumerate_data((j), &(data), &(l))) > 0; )

Journal::Journal(QObject *parent) : QObject(parent)
{

}

void Journal::addMatch(const QString &match)
{
    if (sdj) {
        sd_journal_add_match(sdj, match.toUtf8().constData(), 0);
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
            if (size <= 0) {
                sd_journal_previous(sdj);
            } else {
                sd_journal_previous_skip(sdj, (uint64_t)size);
            }
        }
    }
}

void Journal::seekTimestamp(quint64 timestamp)
{
    if (sdj) {
        sd_journal_seek_realtime_usec(sdj, (uint64_t)timestamp * 1000);
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

    skipTail(50);

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
            if (sd_journal_get_realtime_usec(sdj, &realtime) < 0) {
                break;
            }

            jsonData["__TIMESTAMP"] = realtime / 1000;

            emit dataReceived(jsonData);
        } else if (next_ret == 0) {
            if (sd_journal_wait(sdj, (uint64_t)-1) < 0) {
                break;
            }
        } else if (next_ret < 0) {
            break;
        }
    }

    sd_journal_close(sdj);

    emit quit();
}
