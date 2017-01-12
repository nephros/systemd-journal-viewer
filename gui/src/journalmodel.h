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

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const { return m_roles; }

    Q_INVOKABLE void skipTail(int count);
    Q_INVOKABLE void addMatch(const QString & match);
    Q_INVOKABLE void flushMatches();

    Q_INVOKABLE void clear();

    Q_INVOKABLE void save(const QString & filePath);

    Q_INVOKABLE void ping();
    Q_INVOKABLE void quit();

private slots:
    void onDataReceived(const QVariantMap & data);

private:
    QStringList m_keys;
    QHash<int, QByteArray> m_roles;
    QList<QVariantMap> m_modelData;

    QDBusInterface *m_iface;
};

#endif // JOURNALMODEL_H
