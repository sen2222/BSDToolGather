#include "getimgviewright.h"
#include "ui_getimgviewright.h"

#include "alltoolfun.h"

GetImgViewRight::GetImgViewRight(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgViewRight)
{
    ui->setupUi(this);
    QString curPath = QDir::currentPath();
    QString IRSavePathTmp = CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_RIGHT, CONFIG_KEY_DEFAULT_IR_SAVE_PATH, "yuv");
    QString RGBSavePathTmp = CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_RIGHT, CONFIG_KEY_DEFAULT_RGB_SAVE_PATH, "yuv");
    QString BMPSavePathTmp = CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_RIGHT, CONFIG_KEY_DEFAULT_BMP_SAVE_PATH, "bmp");
    QString JPEGSavePathTmp = CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_RIGHT, CONFIG_KEY_DEFAULT_JPEG_SAVE_PATH, "jpeg");
    QString PNGSavePathTmp = CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_RIGHT, CONFIG_KEY_DEFAULT_PNG_SAVE_PATH, "png");
    savePath.IRSavePath = curPath + "/" + IRSavePathTmp;
    savePath.RGBSavePath = curPath + "/" + RGBSavePathTmp;
    savePath.BMPSavePath = curPath + "/" + BMPSavePathTmp;
    savePath.JPEGSavePath = curPath + "/" + JPEGSavePathTmp;
    savePath.PNGSavePath = curPath + "/" + PNGSavePathTmp;
    savePath.rightView = this;
    saveIRNum = 0;
    saveRGBNum = 0;
    setSaveStatus(1, SYSTEM_STATUS_CLOSE);
    setSaveStatus(2, SYSTEM_STATUS_CLOSE);
    setSaveBtnStatus(1, getSaveStatus(1));
    setSaveBtnStatus(2, getSaveStatus(2));
    setLedStatus(1, SYSTEM_STATUS_CLOSE);
    setLedStatus(2, SYSTEM_STATUS_CLOSE);
    setIrcutStatus(1, SYSTEM_STATUS_CLOSE);
    setIrcutStatus(2, SYSTEM_STATUS_CLOSE);
}

GetImgViewRight::~GetImgViewRight()
{
    delete getImgAppSaveFrame1;
    delete getImgAppSaveFrame2;
    delete ui;
}

void GetImgViewRight::setLeftView(GetImgViewLeft *getImgViewLeft)
{
    this->getImgViewLeft = getImgViewLeft;
    connect(getImgViewLeft, &GetImgViewLeft::socketUpdateConnectStatus, this, &GetImgViewRight::socketUpdateConnectStatusSlot);
    connect(getImgViewLeft, &GetImgViewLeft::socketUpdateLedStatus, this, &GetImgViewRight::socketUpdateLedStatusSlot);
    connect(getImgViewLeft, &GetImgViewLeft::socketUpdateIrcutStatus, this, &GetImgViewRight::socketUpdateIrcutStatusSlot);
}



void GetImgViewRight::SaveThreadInit(void)
{
    getImgAppSaveFrame1 = new GetImgAppSaveFrame(savePath);
    getImgAppSaveFrame2 = new GetImgAppSaveFrame(savePath);

    BSD_LOG(LOG_INFO, "get img save thread init success\n");
}
void GetImgViewRight::SaveThreadDeinit(void)
{
    if(getImgAppSaveFrame1 != nullptr)
        delete getImgAppSaveFrame1;
    if(getImgAppSaveFrame2 != nullptr)
        delete getImgAppSaveFrame2;
    
    getImgAppSaveFrame1 = nullptr;
    getImgAppSaveFrame2 = nullptr;
    BSD_LOG(LOG_INFO, "get img save thread deinit success\n");
}

void GetImgViewRight::setSaveStatus(int ipId, SYSTEM_STATUS_E saveStatus)
{
    QMutexLocker locker(&saveStateMutex);
    if(ipId == 1)
        ip1SaveStatus = saveStatus;
    else if(ipId == 2)
        ip2SaveStatus = saveStatus;
}

SYSTEM_STATUS_E GetImgViewRight::getSaveStatus(int ipId)
{
    QMutexLocker locker(&saveStateMutex);
    if(ipId == 1)
        return ip1SaveStatus;
    else if(ipId == 2)
        return ip2SaveStatus;
    return SYSTEM_STATUS_CLOSE;
}
void GetImgViewRight::setLedStatus(int ipId, SYSTEM_STATUS_E ledStatus)
{
    QMutexLocker locker(&ledMutex);
    if(ipId == 1)
    {
        ip1LedStatus = ledStatus;
        ledStatus == SYSTEM_STATUS_OPEN ? ui->btnLedSwitch1->setText(QString("IP%1 关闭LED").arg(ipId)) : ui->btnLedSwitch1->setText(QString("IP%1 开启LED").arg(ipId));
    }
    else if(ipId == 2)
    {
        ip2LedStatus = ledStatus;
        ledStatus == SYSTEM_STATUS_OPEN ? ui->btnLedSwitch2->setText(QString("IP%1 关闭LED").arg(ipId)) : ui->btnLedSwitch2->setText(QString("IP%1 开启LED").arg(ipId));
    }
}
SYSTEM_STATUS_E GetImgViewRight::getLedStatus(int ipId)
{
    QMutexLocker locker(&ledMutex);
    if(ipId == 1)
        return ip1LedStatus;
    else if(ipId == 2)
        return ip2LedStatus;
    return SYSTEM_STATUS_CLOSE;
}
void GetImgViewRight::setIrcutStatus(int ipId, SYSTEM_STATUS_E ircutStatus)
{
    QMutexLocker locker(&ircutMutex);
    if(ipId == 1)
    {
        ip1IrcutStatus = ircutStatus;
        ircutStatus == SYSTEM_STATUS_OPEN ? ui->btnIrcutSwitch1->setText(QString("IP%1 关闭IRUT").arg(ipId)) : ui->btnIrcutSwitch1->setText(QString("IP%1 开启IRUT").arg(ipId));
    }
    else if(ipId == 2)
    {
        ip2IrcutStatus = ircutStatus;
        ircutStatus == SYSTEM_STATUS_OPEN ? ui->btnIrcutSwitch2->setText(QString("IP%1 关闭IRUT").arg(ipId)) : ui->btnIrcutSwitch2->setText(QString("IP%1 开启IRUT").arg(ipId));
    }
}
SYSTEM_STATUS_E GetImgViewRight::getIrcutStatus(int ipId)
{
    QMutexLocker locker(&ircutMutex);
    if(ipId == 1)
        return ip1IrcutStatus;
    else if(ipId == 2)
        return ip2IrcutStatus;
    return SYSTEM_STATUS_CLOSE;
}


uint32_t GetImgViewRight::getImgSaveNum(IMG_SRC_TYPE_E imgSrcType)
{
    QMutexLocker locker(&saveNumMutex);
    if(imgSrcType == IMG_SRC_IR)
        return saveIRNum;
    else if(imgSrcType == IMG_SRC_RGB)
        return saveRGBNum;
    return 0;
}
void GetImgViewRight::addImgSaveNum(IMG_SRC_TYPE_E imgSrcType)
{
    QMutexLocker locker(&saveNumMutex);
    if(imgSrcType == IMG_SRC_IR)
    {
        saveIRNum++;
        ui->labelIRNum->setText(QString::number(saveIRNum));
    }
    else if(imgSrcType == IMG_SRC_RGB)
    {
        saveRGBNum++;
        ui->labelRGBNum->setText(QString::number(saveRGBNum));
    }
}
void GetImgViewRight::clearImgSaveNum(void)
{
    QMutexLocker locker(&saveNumMutex);
    saveIRNum = 0;
    saveRGBNum = 0;
    ui->labelIRNum->setText(QString::number(saveIRNum));
    ui->labelRGBNum->setText(QString::number(saveRGBNum));
}

void GetImgViewRight::setSaveBtnStatus(int ipId, SYSTEM_STATUS_E saveStatus)
{
    QPushButton *btn = nullptr;
    btn = (ipId == 1) ? ui->btnIP1Save : ui->btnIP2Save;
    if(saveStatus == SYSTEM_STATUS_CLOSE)
    {
        btn->setText(QString("IP%1 开启保存").arg(ipId));
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #2ECC71;
                color: #FFFFFF;
                border: 1px solid #27AE60;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:pressed {
                background-color: #27AE60;
                border: 1px solid #219653;
            }
        )");
    }
    else
    {
        btn->setText(QString("IP%1 暂停保存").arg(ipId));
        btn->setStyleSheet(R"(
                QPushButton {
                background-color: #E74C3C;
                color: #FFFFFF;
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:pressed {
                background-color: #C0392B;
            }
        )");
    }
}
void GetImgViewRight::on_btnIP1Save_clicked()
{
    if(ip1SaveStatus == 0)
    {
        getImgViewLeft->getImgStartSave(getImgAppSaveFrame1, nullptr);
        ip1SaveStatus = SYSTEM_STATUS_OPEN;
        setSaveBtnStatus(1, getSaveStatus(1));
        BSD_LOG_INFO("ip1 开始保存YUV帧\n");
    }
    else
    {
        getImgViewLeft->getImgStopSave(1);
        ip1SaveStatus = SYSTEM_STATUS_CLOSE;
        setSaveBtnStatus(1, getSaveStatus(1));
        BSD_LOG_INFO("ip1 暂停保存YUV帧\n");
    }
}

void GetImgViewRight::on_btnIP2Save_clicked()
{
    if(ip2SaveStatus == 0)
    {
        getImgViewLeft->getImgStartSave(nullptr, getImgAppSaveFrame2);
        ip2SaveStatus = SYSTEM_STATUS_OPEN;
        setSaveBtnStatus(2, getSaveStatus(2));
        BSD_LOG_INFO("ip2 开始保存YUV帧\n");
    }
    else
    {
        getImgViewLeft->getImgStopSave(2);
        ip2SaveStatus = SYSTEM_STATUS_CLOSE;
        setSaveBtnStatus(2, getSaveStatus(2));
        BSD_LOG_INFO("ip2 暂停保存YUV帧\n");
    }
}


void GetImgViewRight::on_btnClearNum_clicked()
{
    clearImgSaveNum();
}

void GetImgViewRight::on_btnPacket_clicked()
{
    getImgAppSaveFrame1->packCurrentSaveDir();
    getImgAppSaveFrame2->packCurrentSaveDir();
    clearImgSaveNum();
}


void GetImgViewRight::on_boxI2d_currentIndexChanged(int index)
{
    getImgViewLeft->getImgSetShowAngle(index);
}


void GetImgViewRight::on_boxSaveDedoder_currentIndexChanged(int index)
{
    getImgViewLeft->getImgSetSaveType(index);
}


void GetImgViewRight::socketUpdateConnectStatusSlot(int id, SOCKET_CONNECT_STATUS_E status)
{
    if(id == 1)
    {
        ui->btnLedSwitch1->setEnabled(status == SOCKET_CONNECT);
        ui->btnIrcutSwitch1->setEnabled(status == SOCKET_CONNECT);
    }else if(id == 2)
    {
        ui->btnLedSwitch2->setEnabled(status == SOCKET_CONNECT);
        ui->btnIrcutSwitch2->setEnabled(status == SOCKET_CONNECT);
    }
    setLedStatus(id, SYSTEM_STATUS_CLOSE);
    setIrcutStatus(id, SYSTEM_STATUS_CLOSE);
}

void GetImgViewRight::socketUpdateLedStatusSlot(int id, SYSTEM_STATUS_E status)
{
    setLedStatus(id, status);
}
void GetImgViewRight::socketUpdateIrcutStatusSlot(int id, SYSTEM_STATUS_E status)
{
    setIrcutStatus(id, status);
}

void GetImgViewRight::on_btnLedSwitch1_clicked()
{
    getImgViewLeft->getImgSetLedStatus(1, getLedStatus(1) == SYSTEM_STATUS_CLOSE ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
}
void GetImgViewRight::on_btnLedSwitch2_clicked()
{
    getImgViewLeft->getImgSetLedStatus(2, getLedStatus(2) == SYSTEM_STATUS_CLOSE ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
}
void GetImgViewRight::on_btnIrcutSwitch1_clicked()
{
    getImgViewLeft->getImgSetIrcutStatus(1, getIrcutStatus(1) == SYSTEM_STATUS_CLOSE ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
}
void GetImgViewRight::on_btnIrcutSwitch2_clicked()
{
    getImgViewLeft->getImgSetIrcutStatus(2, getIrcutStatus(2) == SYSTEM_STATUS_CLOSE ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
}

