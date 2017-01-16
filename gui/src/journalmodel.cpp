#include "journalmodel.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

JournalModel::JournalModel(QObject *parent):
    QAbstractListModel(parent)
{
    m_iface = new QDBusInterface("ru.omprussia.systemd.journal", "/", "ru.omprussia.systemd.journal", QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect("ru.omprussia.systemd.journal", "/", "ru.omprussia.systemd.journal", "dataReceived", this, SLOT(onDataReceived(QVariantMap)));
}

int JournalModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_modelData.count();
}

QVariant JournalModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_modelData.count())
        return QVariantMap();
    return m_modelData[m_modelData.count() - row - 1][m_roles[role]];
}

void JournalModel::skipTail(int count)
{
    m_iface->call(QDBus::NoBlock, "skipTail", count);
    clear();
}

void JournalModel::seekTimestamp(quint64 timestamp)
{
    m_iface->call(QDBus::NoBlock, "seekTimestamp", timestamp);
    clear();
}

void JournalModel::addMatch(const QString &match)
{
    m_iface->call(QDBus::NoBlock, "addMatch", match);
}

void JournalModel::flushMatches()
{
    m_iface->call(QDBus::NoBlock, "flushMatches");
}

void JournalModel::copyItem(int index)
{
    if (index < 0 || index >= m_modelData.count()) {
        return;
    }

    qGuiApp->clipboard()->setText(logItem(m_modelData[index]));
}

void JournalModel::clear()
{
    beginResetModel();
    m_modelData.clear();
    endResetModel();
}

void JournalModel::save(const QString &filePath)
{
    QString fileName = QString("%1/systemd-journal-%2.txt").arg(filePath).arg(QDateTime::currentDateTime().toString("ddMMyy_hhmmss-zzz"));
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        foreach (const QVariantMap & data, m_modelData) {
            out << logItem(data) << endl;
        }
        file.close();
    }
}

void JournalModel::ping()
{
    m_iface->call(QDBus::NoBlock, "ping");
}

void JournalModel::quit()
{
    m_iface->call(QDBus::NoBlock, "quit");
}

void JournalModel::onDataReceived(const QVariantMap &data)
{
    foreach (const QString & key, data.keys()) {
        if (!m_keys.contains(key)) {
            m_keys.append(key);
            m_roles[Qt::UserRole + m_keys.count()] = key.toLatin1();
        }
    }
    beginInsertRows(QModelIndex(), 0, 0);
    m_modelData.append(data);
    endInsertRows();
}

QString JournalModel::logItem(const QVariantMap & data)
{
    return QString("%1 %2: %3%4").arg(QDateTime::fromMSecsSinceEpoch(data["__TIMESTAMP"].toULongLong()).toString("dd.MM.yy hh:mm:ss.zzz"))
                                 .arg(data.contains("_PID") ? QString("%1[%2]").arg(data["SYSLOG_IDENTIFIER"].toString()).arg(data["_PID"].toString())
                                                            : data["SYSLOG_IDENTIFIER"].toString())
                                 .arg(data.contains("CODE_FILE") ? QString("%1:%2").arg(data["CODE_FILE"].toString()).arg(data["CODE_LINE"].toString())
                                                                 : QString())
                                 .arg(data["MESSAGE"].toString());
}
