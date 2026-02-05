#ifndef MAINLOGWIDGET_H
#define MAINLOGWIDGET_H

#include <QWidget>
#include "logoutput.h"


#define LOG_MAX_LINES 100

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
