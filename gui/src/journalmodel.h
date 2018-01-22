#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

#include <QAbstractListModel>
#include <QtDBus>
#include <QObject>

class JournalModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap lastEntry READ lastEntry NOTIFY lastEntryChanged)
public:
    explicit JournalModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const { return m_roles; }

    QVariantMap lastEntry() const;

signals:
    void lastEntryChanged();

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
    void onDataReceived(const QVariantList &data);

private:
    QString logItem(const QVariantMap &data);
    QVariant unwind(const QVariant &val, int depth = 0);

    QStringList m_keys;
    QHash<int, QByteArray> m_roles;
    QList<QVariantMap> m_modelData;

    QDBusInterface *m_iface;
};

#endif // JOURNALMODEL_H
