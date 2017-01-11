#include "journal.h"

#include <QVariantMap>
#include <stdio.h>
#include <systemd/sd-journal.h>

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

void Journal::init()
{
    int         ret;
    sd_journal	*sdj;

    if ((ret = sd_journal_open(&sdj, SD_JOURNAL_LOCAL_ONLY)) < 0) {
        emit quit();
        return;
    }

    ret = sd_journal_get_fd(sdj);
    if (ret < 0) {
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
        const void *data;
        size_t length;
        int next_ret;
        next_ret = sd_journal_next(sdj);
        if (next_ret == 0) {
            ret = sd_journal_wait(sdj, (uint64_t)-1);
        }
        if (next_ret == 0) {
            continue;
        }

        QVariantMap jsonData;

        JOURNAL_FOREACH_DATA_RETVAL(sdj, data, length, ret) {
            QString fieldLine = QString::fromUtf8((const char*)data, length);
            int fieldIndex = fieldLine.indexOf("=");
            QString fieldName = fieldLine.left(fieldIndex);
            QString fieldData = fieldLine.mid(fieldIndex + 1);
            jsonData[fieldName] = fieldData;
        }

        uint64_t realtime;
        ret = sd_journal_get_realtime_usec(sdj, &realtime);
        if (ret == 0) {
            jsonData["__TIMESTAMP"] = realtime;
        }

        emit dataReceived(jsonData);
    }

    sd_journal_close(sdj);

    return;
}
