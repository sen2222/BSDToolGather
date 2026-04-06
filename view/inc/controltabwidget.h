#ifndef CONTROLTABWIDGET_H
#define CONTROLTABWIDGET_H

#include <QWidget>

namespace Ui {
class ControlTabWidget;
}

class ControlTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlTabWidget(QWidget *parent = nullptr);
    ~ControlTabWidget();

private slots:
    void on_tabWidget_currentChanged(int index);

private:
    Ui::ControlTabWidget *ui;
    int currentTabIndex;
};

#endif // CONTROLTABWIDGET_H
