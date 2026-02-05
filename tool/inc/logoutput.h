#ifndef LOGOUTPUT_H
#define LOGOUTPUT_H

#include <QFile>
#include <QDate>
#include <QObject>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

#define CONFIG_SECTION_LOG_OUTPUT   "LogOutput"
#define CONFIG_KEY_LOG_LEVEL        "logLevel"
#define CONFIG_KEY_LOG_FILE_PATH    "logFileDir"

typedef enum
{
    LOG_ERRO = 0,
    LOG_WARN,
    LOG_INFO,
    LOG_DBUG,
} LOG_LEVEL_E;

class LogOutput : public QObject
{
    Q_OBJECT
public:
    static LogOutput* getInstance()
    {
        static LogOutput instance;
        return &instance;
    }
    
    LogOutput(const LogOutput&) = delete;
    LogOutput& operator=(const LogOutput&) = delete;
    QString logOutGetLevelName(LOG_LEVEL_E level);
    void logOutToView(LOG_LEVEL_E level, const QString& message);
    void logOutToFile(LOG_LEVEL_E level, const QString& message);
    void logOut(LOG_LEVEL_E level, const QString& message);
    void logOutInfoToView(const QString& message);
    void logOutputSetLevel(LOG_LEVEL_E level);
    int logOutputGetLevel();
    
signals:
    void logOutToViewSlot(LOG_LEVEL_E level, QString message);

private:
    explicit LogOutput(QObject* parent = nullptr);
    ~LogOutput();
        
    int logLevel;
    QString logFilePath;    
    QString logDir;         
    QFile* logFile;
    QDate currentLogDate;   
    QMutex m_logMutex;
};

#endif