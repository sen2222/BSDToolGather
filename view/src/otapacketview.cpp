#include "otapacketview.h"
#include "ui_otapacketview.h"

OtaPacketView::OtaPacketView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtaPacketView)
{
    ui->setupUi(this);
    ui->rightWidget->setLeftView(ui->leftWidget);
}

OtaPacketView::~OtaPacketView()
{
    delete ui;
}
