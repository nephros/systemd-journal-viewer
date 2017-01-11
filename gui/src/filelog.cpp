#include "filelog.h"

#include <QStandardPaths>

FileLog::FileLog(QObject *parent) : QObject(parent)
{

}

FileLog::~FileLog()
{
    if (m_file->isOpen()) {
        m_file->flush();
        m_file->close();
    }
}

QString FileLog::fileName() const
{
    return m_fileName;
}

void FileLog::setFileName(const QString &fileName)
{
    if (m_fileName != fileName) {
        m_fileName = fileName;
        emit fileNameChanged(fileName);
    }
}

void FileLog::classBegin()
{

}

void FileLog::componentComplete()
{
    if (!m_fileName.isEmpty()) {
        QString fileName = QString("%1/%2.txt").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).arg(m_fileName);
        m_file = new QFile(fileName, this);
        m_file->open(QFile::WriteOnly) | QFile::Text;
        m_stream.setDevice(m_file);
    }
}

void FileLog::writeLine(const QString &line)
{
    if (m_file->isOpen()) {
        m_stream << line << endl;
    }
}
