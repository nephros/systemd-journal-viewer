#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

#include <QAbstractListModel>
#include <QtDBus>
#include <QObject>

class JournalModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit JournalModel(QObject *parent = 0);

    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const { return m_roles; }

public slots:
    void skipTail(int count);
    void seekTimestamp(quint64 timestamp);
    void addMatch(const QString & match);
    void flushMatches();

    void copyItem(int index);
    void clear();

    void save(const QString & filePath);

    void ping();
    void quit();

private slots:
    void onDataReceived(const QVariantMap & data);

private:
    QString logItem(const QVariantMap &data);

    QStringList m_keys;
    QHash<int, QByteArray> m_roles;
    QList<QVariantMap> m_modelData;

    QDBusInterface *m_iface;
};

#endif // JOURNALMODEL_H
