#ifndef JOURNAL_H
#define JOURNAL_H

#include <QObject>
#include <systemd/sd-journal.h>

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
    void dataReceived(const QVariantMap & data);
    void quit();

public slots:
    void init();
};

#endif // JOURNAL_H
