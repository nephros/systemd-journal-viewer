#include "journalmodel.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

JournalModel::JournalModel(QObject *parent):
    QAbstractListModel(parent)
{
    m_iface = new QDBusInterface("ru.omprussia.systemd.journal", "/", "ru.omprussia.systemd.journal", QDBusConnection::sessionBus(), this);
    QDBusConnection::sessionBus().connect("ru.omprussia.systemd.journal", "/", "ru.omprussia.systemd.journal", "dataReceived", this, SLOT(onDataReceived(QVariantList)));
}

int JournalModel::rowCount(const QModelIndex &) const
{
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

void JournalModel::onDataReceived(const QVariantList &data)
{
    beginInsertRows(QModelIndex(), 0, data.length() - 1);
    const QVariantList finalData = unwind(data).toList();
    for (const QVariant &dataItem : finalData) {
        const QVariantMap &journalData = dataItem.toMap();
        for (const QString &key : journalData.keys()) {
            if (!m_keys.contains(key)) {
                m_keys.append(key);
                m_roles[Qt::UserRole + m_keys.count()] = key.toLatin1();
            }
        }
        m_modelData.append(journalData);
    }
    endInsertRows();
}

QVariant JournalModel::unwind(const QVariant &val, int depth)
{
    /* Limit recursion depth to protect against type conversions
     * that fail to converge to basic qt types within qt variant.
     *
     * Using limit >= DBUS_MAXIMUM_TYPE_RECURSION_DEPTH (=32) should
     * mean we do not bail out too soon on deeply nested but othewise
     * valid dbus messages. */
    static const int maximum_dept = 32;

    /* Default to QVariant with isInvalid() == true */
    QVariant res;

    const int type = val.userType();

    if( ++depth > maximum_dept ) {
        /* Leave result to invalid variant */
        qWarning() << "Too deep recursion detected at userType:" << type;
    }
    else if (type == QVariant::List) {
        /* Is built-in type, but does not get correctly converted
         * to qml domain if contains QDBus types inside -> convert
         * to variant list and unwind each item separately */
        QVariantList list;
        for (const QVariant &var: val.toList()) {
            list.append(unwind(var, depth));
        }
        res = list;
    }
    else if (type == QVariant::ByteArray ) {
        /* Is built-in type, but does not get correctly converted
         * to qml domain -> convert to variant list */
        QByteArray arr = val.toByteArray();
        QVariantList lst;
        for( int i = 0; i < arr.size(); ++i )
            lst <<QVariant::fromValue(static_cast<quint8>(arr[i]));
        res = QVariant::fromValue(lst);
    }
    else if (type == val.type()) {
        /* Already is built-in qt type, use as is */
        res = val;
    } else if (type == qMetaTypeId<QDBusVariant>()) {
        /* Convert QDBusVariant to QVariant */
        res = unwind(val.value<QDBusVariant>().variant(), depth);
    } else if (type == qMetaTypeId<QDBusObjectPath>()) {
        /* Convert QDBusObjectPath to QString */
        res = val.value<QDBusObjectPath>().path();
    } else if (type == qMetaTypeId<QDBusSignature>()) {
        /* Convert QDBusSignature to QString */
        res =  val.value<QDBusSignature>().signature();
    } else if (type == qMetaTypeId<QDBusUnixFileDescriptor>()) {
        /* Convert QDBusUnixFileDescriptor to int */
        res =  val.value<QDBusUnixFileDescriptor>().fileDescriptor();
    } else if (type == qMetaTypeId<QDBusArgument>()) {
        /* Try to deal with everything QDBusArgument could be ... */
        const QDBusArgument &arg = val.value<QDBusArgument>();
        const QDBusArgument::ElementType elem = arg.currentType();
        switch (elem) {
        case QDBusArgument::BasicType:
            /* Most of the basic types should be convertible to QVariant.
             * Recurse anyway to deal with object paths and the like. */
            res = unwind(arg.asVariant(), depth);
            break;

        case QDBusArgument::VariantType:
            /* Try to convert to QVariant. Recurse to check content */
            res = unwind(arg.asVariant().value<QDBusVariant>().variant(),
                         depth);
            break;

        case QDBusArgument::ArrayType:
            /* Convert dbus array to QVariantList */
            {
                QVariantList list;
                arg.beginArray();
                while (!arg.atEnd()) {
                    QVariant tmp = arg.asVariant();
                    list.append(unwind(tmp, depth));
                }
                arg.endArray();
                res = list;
            }
            break;

        case QDBusArgument::StructureType:
            /* Convert dbus struct to QVariantList */
            {
                QVariantList list;
                arg.beginStructure();
                while (!arg.atEnd()) {
                    QVariant tmp = arg.asVariant();
                    list.append(unwind(tmp, depth));
                }
                arg.endStructure();
                res = QVariant::fromValue(list);
            }
            break;

        case QDBusArgument::MapType:
            /* Convert dbus dict to QVariantMap */
            {
                QVariantMap map;
                arg.beginMap();
                while (!arg.atEnd()) {
                    arg.beginMapEntry();
                    QVariant key = arg.asVariant();
                    QVariant val = arg.asVariant();
                    map.insert(unwind(key, depth).toString(),
                               unwind(val, depth));
                    arg.endMapEntry();
                }
                arg.endMap();
                res = map;
            }
            break;

        default:
            /* Unhandled types produce invalid QVariant */
            qWarning() << "Unhandled QDBusArgument element type:" << elem;
            break;
        }
    } else {
        /* Default to using as is. This should leave for example QDBusError
         * types in a form that does not look like a string to qml code. */
        res = val;
        qWarning() << "Unhandled QVariant userType:" << type;
    }

    return res;
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
