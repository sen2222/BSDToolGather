#include "getimgview.h"
#include "ui_getimgview.h"

GetImgView::GetImgView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgView)
{
    ui->setupUi(this);
    ui->rightWidget->setLeftView(ui->leftWidget);
}

GetImgView::~GetImgView()
{
    delete ui;
}


void GetImgView::leftWidgetInit(void)
{
    ui->leftWidget->socketImgInit();
}
void GetImgView::leftWidgetDeinit(void)
{
    ui->leftWidget->socketImgDeinit();
}

void GetImgView::rightWidgetInit(void)
{
    ui->rightWidget->SaveThreadInit();
}
void GetImgView::rightWidgetDeinit(void)
{
    ui->rightWidget->SaveThreadDeinit();
}
