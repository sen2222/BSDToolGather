#include "controltabwidget.h"
#include "ui_controltabwidget.h"

ControlTabWidget::ControlTabWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlTabWidget)
{
    ui->setupUi(this);
    currentTabIndex = ui->tabWidget->currentIndex();
    if(currentTabIndex == 1)
        ui->tabGetImg->leftWidgetInit();
}

ControlTabWidget::~ControlTabWidget()
{
    if (currentTabIndex == 1)
        ui->tabGetImg->leftWidgetDeinit();
    delete ui;
}

void ControlTabWidget::on_tabWidget_currentChanged(int index)
{

    if (currentTabIndex == 1 && index != 1) {
        ui->tabGetImg->leftWidgetDeinit();
        ui->tabGetImg->rightWidgetDeinit();
    }
    if (index == 1 && currentTabIndex != 1) {
        ui->tabGetImg->leftWidgetInit();
        ui->tabGetImg->rightWidgetInit();
    }

    currentTabIndex = index;
}