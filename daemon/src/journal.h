#ifndef JOURNAL_H
#define JOURNAL_H

#include <QObject>
#include <systemd/sd-journal.h>
#include <QCoreApplication>
#include <QThread>
#include <QDateTime>

class JournalWaiter : public QObject
{
    Q_OBJECT
public:
    explicit JournalWaiter(sd_journal *sdj) : QObject(nullptr) {
        this->sdj = sdj;
    }

public slots:
    void start() {
        int ret = sd_journal_wait(sdj, (uint64_t)-1);
        if (ret < 0) {
            emit pollFailed();
        } else if (ret == SD_JOURNAL_NOP) {
            emit pollAgain();
        } else {
            emit canProcess();
        }

        deleteLater();
    }

private:
    sd_journal *sdj;

signals:
    void pollFailed();
    void canProcess();
    void pollAgain();
};

class Journal : public QObject
{
    Q_OBJECT
public:
    explicit Journal(QObject *parent = 0);

public slots:
    void addMatch(const QString & match);
    void flushMatches();
    void skipTail(int size);
    void seekTimestamp(quint64 timestamp);

private:
    sd_journal *sdj;

signals:
    void dataReceived(const QVariantList & data);
    void quit();

public slots:
    void init();
    void process();

private slots:
    void wait();
};

#endif // JOURNAL_H
