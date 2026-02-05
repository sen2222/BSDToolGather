#include "getimgview.h"
#include "ui_getimgview.h"

GetImgView::GetImgView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgView)
{
    ui->setupUi(this);
}

GetImgView::~GetImgView()
{
    delete ui;
}
