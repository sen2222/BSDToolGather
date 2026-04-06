#ifndef MAINLOGWIDGET_H
#define MAINLOGWIDGET_H

#include <QWidget>
#include "logoutput.h"


#define LOG_MAX_LINES 100

#define CONFIG_SECTION_SYSTEM_INFO "SystemInfo"
#define CONFIG_KEY_EMAIL            "Email"
#define CONFIG_KEY_VERSION          "Version"   


namespace Ui {
class MainLogWidget;
}

class MainLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainLogWidget(QWidget *parent = nullptr);
    ~MainLogWidget();

public slots:
    void onLogMessageReceived(LOG_LEVEL_E level, const QString message);

private:
    Ui::MainLogWidget *ui;
};

#endif // MAINLOGWIDGET_H
