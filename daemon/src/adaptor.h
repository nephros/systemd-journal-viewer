#ifndef ADAPTOR_H
#define ADAPTOR_H

#include "journal.h"

#include <QObject>

class Adaptor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ru.omprussia.systemd.journal")
public:
    explicit Adaptor(QObject *parent = 0);

public slots:
    void start();

private:
    Journal *m_journal;

signals:
    Q_SCRIPTABLE void dataReceived(const QVariantMap & data);

    void doAddMatch(const QString & match);
    void doFlushMatches();
    void doSkipTail(int size);
    void doSeekTimestamp(quint64 timestamp);

public slots:
    Q_SCRIPTABLE void ping();
    Q_SCRIPTABLE void quit();

    Q_SCRIPTABLE void addMatch(const QString & match);
    Q_SCRIPTABLE void flushMatches();
    Q_SCRIPTABLE void skipTail(int size);
    Q_SCRIPTABLE void seekTimestamp(quint64 timestamp);
};

#endif // ADAPTOR_H
