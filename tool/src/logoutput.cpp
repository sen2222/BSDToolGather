#include "logoutput.h"
#include "alltoolfun.h"

LogOutput::LogOutput(QObject* parent)
    : QObject(parent)
{
    QString logDirTmp = CONFIG_READ_STRING(CONFIG_SECTION_LOG_OUTPUT, CONFIG_KEY_LOG_FILE_PATH, "/log");
    logLevel = CONFIG_READ_INT(CONFIG_SECTION_LOG_OUTPUT, CONFIG_KEY_LOG_LEVEL, 0);
    logDir = QDir::currentPath() + "/" + logDirTmp;
    
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentLogDate = currentDateTime.date();
    QString logFileName = QString("LOG-%1.log").arg(currentLogDate.toString("yyyy-MMdd"));
    logFilePath = dir.filePath(logFileName);

    logFile = new QFile(logFilePath);
    if (!logFile->open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Failed to open log file:" << logFilePath;
    } else {
        qDebug() << "Log file opened:" << logFilePath;
    }    
    qDebug() << "Log level:" << logLevel;
}

LogOutput::~LogOutput()
{
    if (logFile) {
        if (logFile->isOpen()) {
            logFile->close();
        }
        delete logFile;
    }
}


QString LogOutput::logOutGetLevelName(LOG_LEVEL_E level)
{
    switch (level)
    {
    case LOG_ERRO:
        return QString("ERRO");
    case LOG_WARN:
        return QString("WARN");
    case LOG_DBUG:
        return QString("DBUG");
    case LOG_INFO:
        return QString("INFO");
    default:
        return QString("unknown");
    }
}

void LogOutput::logOutToView(LOG_LEVEL_E level, const QString& message)
{
    if (level > logLevel) {
        return;
    }
    QMutexLocker locker(&m_logMutex);
    emit logOutToViewSlot(level, message);
}

void LogOutput::logOutToFile(LOG_LEVEL_E level, const QString& message)
{
    if (level > logLevel) {
        return;
    }
    QMutexLocker locker(&m_logMutex);
    
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate nowDate = currentDateTime.date();
    if (nowDate != currentLogDate) {
        if (logFile && logFile->isOpen()) {
            logFile->close();
        }
        currentLogDate = nowDate;
        QString logFileName = QString("LOG-%1.log").arg(currentLogDate.toString("yyyy-MMdd"));
        logFilePath = logDir + "/" + logFileName;
        
        if (!logFile) {
            logFile = new QFile(logFilePath);
        } else {
            logFile->setFileName(logFilePath);
        }
        
        if (!logFile->open(QIODevice::Append | QIODevice::Text)) {
            qDebug() << "Failed to open new log file:" << logFilePath;
            return;
        }
    }
    
    if (!logFile || !logFile->isOpen()) {
        if (!logFile) {
            logFile = new QFile(logFilePath);
        }
        if (!logFile->open(QIODevice::Append | QIODevice::Text)) {
            qDebug() << "Failed to open log file:" << logFilePath;
            return;
        }
    }
    
    QString logEntry = QString("(%1)%2 %3")
        .arg(currentDateTime.toString("hh:mm:ss"))
        .arg((level == LOG_ERRO) ? "[ERRO]" :
             (level == LOG_WARN) ? "[WARN]" :
             (level == LOG_DBUG) ? "[DBUG]" : "[INFO]")
        .arg(message);
    
    QTextStream out(logFile);
    out << logEntry;
    out.flush();
}

void LogOutput::logOut(LOG_LEVEL_E level, const QString& message)
{
    logOutToView(level, message);
    logOutToFile(level, message);
}
void LogOutput::logOutInfoToView(const QString& message)
{
    QMutexLocker locker(&m_logMutex);
    emit logOutToViewSlot(LOG_INFO, message);
}

void LogOutput::logOutputSetLevel(LOG_LEVEL_E level)
{
    QMutexLocker locker(&m_logMutex);
    logLevel = level;
    CONFIG_WRITE_INT(CONFIG_SECTION_LOG_OUTPUT, CONFIG_KEY_LOG_LEVEL, level);
}

int LogOutput::logOutputGetLevel()
{
    return logLevel;
}