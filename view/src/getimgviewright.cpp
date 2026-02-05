#include "getimgviewright.h"
#include "ui_getimgviewright.h"

GetImgViewRight::GetImgViewRight(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgViewRight)
{
    ui->setupUi(this);
}

GetImgViewRight::~GetImgViewRight()
{
    delete ui;
}
