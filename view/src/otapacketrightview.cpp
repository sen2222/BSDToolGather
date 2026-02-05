#include "otapacketrightview.h"
#include "ui_otapacketrightview.h"

OtaPacketRightView::OtaPacketRightView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtaPacketRightView)
{
    ui->setupUi(this);
    m_packetApp = new OtaPacketApp();
}

OtaPacketRightView::~OtaPacketRightView()
{
    delete ui;
    delete m_packetApp;
}

void OtaPacketRightView::setLeftView(OtaPacketLeftView* leftView)
{
    m_leftView = leftView;
}
void OtaPacketRightView::clearAllUserParam()
{
    ui->lineEditProjectName->clear();
    ui->lineEditCustimerName->clear();
    ui->spinBoxMajor->setValue(0);
    ui->spinBoxMinor->setValue(0);
    ui->spinBoxPatch->setValue(0);
    ui->cbAlignType->setCurrentIndex(0);
}



void OtaPacketRightView::on_btnPacket_clicked()
{
    OTA_PACKET_PARAM_S param;
    param.projectName = ui->lineEditProjectName->text();
    param.customerName = ui->lineEditCustimerName->text();
    param.major = ui->spinBoxMajor->value();
    param.minor = ui->spinBoxMinor->value();
    param.patch = ui->spinBoxPatch->value();
    param.aligin = ui->cbAlignType->currentIndex();

    m_leftView->getAllUsetParam(param.packetPath, param.isCheck, param.partitionType);
    m_packetApp->packetProcess(&param);

    // m_leftView->clearAllLineEdit();
    // m_leftView->clearAllComboBox();
    // m_leftView->clearAllCheckBox();
    // clearAllUserParam();
}

