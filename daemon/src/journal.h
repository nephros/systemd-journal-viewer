#ifndef JOURNAL_H
#define JOURNAL_H

#include <QObject>

class Journal : public QObject
{
    Q_OBJECT
public:
    explicit Journal(QObject *parent = 0);

signals:
    void dataReceived(const QVariantMap & data);
    void quit();

public slots:
    void init();
};

#endif // JOURNAL_H
