#ifndef FILELOG_H
#define FILELOG_H

#include <QObject>
#include <QQmlParserStatus>
#include <QFile>
#include <QTextStream>

class FileLog : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit FileLog(QObject *parent = 0);
    ~FileLog();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    QString fileName() const;
    void setFileName(const QString & fileName);

    void classBegin();
    void componentComplete();

    Q_INVOKABLE void writeLine(const QString & line);

private:
    QString m_fileName;
    QFile *m_file;
    QTextStream m_stream;

signals:
    void fileNameChanged(const QString & fileName);
};

#endif // FILELOG_H
