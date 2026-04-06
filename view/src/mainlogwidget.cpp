#include "mainlogwidget.h"
#include "ui_mainlogwidget.h"
#include "logoutput.h"
#include "alltoolfun.h"

MainLogWidget::MainLogWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainLogWidget)
{
    QString author  = QString(">作者: %1\n").arg("sent");
    QString email   = QString(">邮箱: %1\n").arg(CONFIG_READ_STRING(CONFIG_SECTION_SYSTEM_INFO, CONFIG_KEY_EMAIL, ""));
    QString version = QString(">版本号: ToolGether-BSD-%1\n").arg(CONFIG_READ_STRING(CONFIG_SECTION_SYSTEM_INFO, CONFIG_KEY_VERSION, ""));
    QString line = QString("-------------------------------------\n");
    ui->setupUi(this);
    ui->planTextLog->appendPlainText(
        line + author + email + version + line
    );
    connect(LogOutput::getInstance(), &LogOutput::logOutToViewSlot, this, &MainLogWidget::onLogMessageReceived);
}

MainLogWidget::~MainLogWidget()
{
    delete ui;
}

void MainLogWidget::onLogMessageReceived(LOG_LEVEL_E level, const QString message)
{
     QColor textColor;
    
    switch(level) {
    case LOG_ERRO:
        textColor = Qt::red;
        break;
    case LOG_WARN:
        textColor = QColor(255, 165, 0);
        break;
    case LOG_DBUG:
        textColor = Qt::blue;
        break;
    case LOG_INFO:
    default:
        textColor = Qt::black;
        break;
    }
    QTextDocument* doc = ui->planTextLog->document();
    int lineCount = doc->blockCount();  // 获取当前行数
    
    if (lineCount >= LOG_MAX_LINES) {
        int linesToDelete = 100;
        if (lineCount - LOG_MAX_LINES + 1 < linesToDelete) {
            linesToDelete = lineCount - LOG_MAX_LINES + 1;
        }
        
        QTextCursor cleanupCursor(doc);
        cleanupCursor.movePosition(QTextCursor::Start);
        cleanupCursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, linesToDelete);
        cleanupCursor.removeSelectedText();
    }
    QTextCursor cursor = ui->planTextLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(textColor);
    cursor.setCharFormat(format);
    
    cursor.insertText(message);
    
    // 滚动到底部
    cursor.movePosition(QTextCursor::End);
    ui->planTextLog->setTextCursor(cursor);
    ui->planTextLog->ensureCursorVisible();
}
