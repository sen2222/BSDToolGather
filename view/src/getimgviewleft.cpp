#include "getimgviewleft.h"
#include "ui_getimgviewleft.h"

#include "alltoolfun.h"

GetImgViewLeft::GetImgViewLeft(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgViewLeft)
{
    ui->setupUi(this);
    ui->labelIR->setMinimumSize(1, 1);
    ui->labelIR->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->labelRGB->setMinimumSize(1, 1);
    ui->labelRGB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->labelIR->setAlignment(Qt::AlignCenter);
    ui->labelIR->setScaledContents(false);
    ui->labelRGB->setAlignment(Qt::AlignCenter);
    ui->labelRGB->setScaledContents(false);
}

GetImgViewLeft::~GetImgViewLeft()
{
    socketImgDeinit();
    delete ui;
}

void GetImgViewLeft::socketImgInit(void)
{
    ip1RingBuffer = new RingBuffer(IP1_RING_BUFFER_SIZE);
    ip2RingBuffer = new RingBuffer(IP2_RING_BUFFER_SIZE);


    videoStatus1 = SYSTEM_STATUS_CLOSE;
    videoStatus2 = SYSTEM_STATUS_CLOSE;
    ui->lineEditIP1->setText(CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_IP1, ""));
    ui->lineEditProt1->setText(CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_PORT1, ""));
    ui->lineEditIP2->setText(CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_IP2, ""));
    ui->lineEditProt2->setText(CONFIG_READ_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_PORT2, ""));

    getImgAppSocket1 = new GetImgAppSocket(1, ip1RingBuffer);
    getImgAppSocket2 = new GetImgAppSocket(2, ip2RingBuffer);
    getImgAppAnalysisFrame1 = new GetImgAppAnalysisFrame(ip1RingBuffer);
    getImgAppAnalysisFrame2 = new GetImgAppAnalysisFrame(ip2RingBuffer);

    connect(getImgAppSocket1, &GetImgAppSocket::connectSuccessSignal, this, &GetImgViewLeft::connectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket1, &GetImgAppSocket::disconnectSuccessSignal, this, &GetImgViewLeft::disconnectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket1, &GetImgAppSocket::socketErrorSignal, this, &GetImgViewLeft::socketErrorSlot, Qt::QueuedConnection);
    connect(getImgAppSocket1, &GetImgAppSocket::sendDataSuccessSignal, this, &GetImgViewLeft::sendDataSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppAnalysisFrame1, &GetImgAppAnalysisFrame::showImgSignal, this, &GetImgViewLeft::showImgSlot, Qt::QueuedConnection);
    connect(getImgAppSocket1, &GetImgAppSocket::analysisDataUpdateSignal, getImgAppAnalysisFrame1, &GetImgAppAnalysisFrame::analysisDataUpdateSlot, Qt::DirectConnection);

    connect(getImgAppSocket2, &GetImgAppSocket::connectSuccessSignal, this, &GetImgViewLeft::connectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::disconnectSuccessSignal, this, &GetImgViewLeft::disconnectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::socketErrorSignal, this, &GetImgViewLeft::socketErrorSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::sendDataSuccessSignal, this, &GetImgViewLeft::sendDataSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppAnalysisFrame2, &GetImgAppAnalysisFrame::showImgSignal, this, &GetImgViewLeft::showImgSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::analysisDataUpdateSignal, getImgAppAnalysisFrame2, &GetImgAppAnalysisFrame::analysisDataUpdateSlot, Qt::DirectConnection);

    BSD_LOG(LOG_INFO, "get img socket init success\n");
}

void GetImgViewLeft::socketImgDeinit(void)
{
    if(getImgAppSocket1)
        delete getImgAppSocket1;
    if(getImgAppSocket2)
        delete getImgAppSocket2;
    if(ip1RingBuffer)
        delete ip1RingBuffer;
    if(ip2RingBuffer)
        delete ip2RingBuffer;
    if(getImgAppAnalysisFrame1)
        delete getImgAppAnalysisFrame1;
    if(getImgAppAnalysisFrame2)
        delete getImgAppAnalysisFrame2;

    getImgAppSocket1 = nullptr;
    getImgAppSocket2 = nullptr;
    getImgAppAnalysisFrame1 = nullptr;
    getImgAppAnalysisFrame2 = nullptr;
    ip1RingBuffer = nullptr;
    ip2RingBuffer = nullptr;

    ui->labelIR->clear();
    ui->labelRGB->clear();

    BSD_LOG(LOG_INFO, "get img socket deinit success\n");
}

void GetImgViewLeft::getImgStartSave(GetImgAppSaveFrame *SaveFrame1, GetImgAppSaveFrame *SaveFrame2)
{
    if(SaveFrame1 != nullptr)
        getImgAppAnalysisFrame1->startSaveFrame(SaveFrame1);
    if(SaveFrame2 != nullptr)
        getImgAppAnalysisFrame2->startSaveFrame(SaveFrame2);
}
void GetImgViewLeft::getImgStopSave(int id)
{
    if(id == 1)
        getImgAppAnalysisFrame1->stopSaveFrame();
    else if(id == 2)
        getImgAppAnalysisFrame2->stopSaveFrame();
}
void GetImgViewLeft::getImgSetShowAngle(int angle)
{
    getImgAppAnalysisFrame1->setShowAngle(angle);
    getImgAppAnalysisFrame2->setShowAngle(angle);
}
void GetImgViewLeft::getImgSetSaveType(int index)
{
    getImgAppAnalysisFrame1->setSaveType(index);
    BSD_LOG_INFO(QString("ip1 设置保存类型为 %1\n").arg(getImgAppAnalysisFrame1->getSaveEncoderChinese(index)));
    getImgAppAnalysisFrame2->setSaveType(index);
    BSD_LOG_INFO(QString("ip2 设置保存类型为 %1\n").arg(getImgAppAnalysisFrame2->getSaveEncoderChinese(index)));
}
void GetImgViewLeft::getImgSetLedStatus(int id, SYSTEM_STATUS_E status)
{
    GetImgAppCmd cmd;
    cmd.setCmd(GET_IMG_CMD_LED);
    cmd.setFlag((status == SYSTEM_STATUS_OPEN) ? GET_IMG_CMD_OPEN : GET_IMG_CMD_CLOSE);
    (id == 1) ? getImgAppSocket1->sendDataToServer(cmd) : getImgAppSocket2->sendDataToServer(cmd);
}
void GetImgViewLeft::getImgSetIrcutStatus(int id, SYSTEM_STATUS_E isOn)
{
    GetImgAppCmd cmd;
    cmd.setCmd(GET_IMG_CMD_IRCUT);
    cmd.setFlag((isOn == SYSTEM_STATUS_OPEN) ? GET_IMG_CMD_OPEN : GET_IMG_CMD_CLOSE);
    (id == 1) ? getImgAppSocket1->sendDataToServer(cmd) : getImgAppSocket2->sendDataToServer(cmd);
}

void GetImgViewLeft::connectSuccessSlot(int id)
{
    QString ipStr;
    QString portStr;
    if(id == 1)
    {
        ip1RingBuffer->clear();
        ui->btnConnect1->setText("disconnect");
        ui->btnOpen1->setText("open");
        ui->btnOpen1->setEnabled(true);
        ipStr = ui->lineEditIP1->text();
        portStr = ui->lineEditProt1->text();
        CONFIG_WRITE_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_IP1, ipStr);
        CONFIG_WRITE_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_PORT1, portStr);
    }
    else if(id == 2)
    {
        ip2RingBuffer->clear();
        ui->btnConnect2->setText("disconnect");
        ui->btnOpen2->setText("open");
        ui->btnOpen2->setEnabled(true);
        ipStr = ui->lineEditIP2->text();
        portStr = ui->lineEditProt2->text();
        CONFIG_WRITE_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_IP2, ipStr);
        CONFIG_WRITE_STRING(CONFIG_SECTION_GET_IMG_VIEW_LEFT, CONFIG_KEY_DEFAULT_PORT2, portStr);
    }
    emit socketUpdateConnectStatus(id, SOCKET_CONNECT);
    BSD_LOG_INFO(QString("Socket连接成功: ID = %1\n").arg(id));
}
void GetImgViewLeft::disconnectSuccessSlot(int id)
{

    if(id == 1)
    {
        ui->btnConnect1->setText("connect");
        ui->btnOpen1->setText("open");
        ui->btnOpen1->setEnabled(false);
        videoStatus1 = SYSTEM_STATUS_CLOSE;
    }
    else if(id == 2)
    {
        ui->btnConnect2->setText("connect");
        ui->btnOpen2->setText("open");
        ui->btnOpen2->setEnabled(false);
        videoStatus2 = SYSTEM_STATUS_CLOSE;
    }
    emit socketUpdateConnectStatus(id, SOCKET_DISCONNECT);
}
void GetImgViewLeft::socketErrorSlot(int id, QAbstractSocket::SocketError socketError)
{
    QString errorStr;
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            errorStr = "连接被拒绝(服务端未启动/端口错误)";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorStr = "主机不存在(IP错误/域名解析失败)";
            break;
        case QAbstractSocket::NetworkError:
            errorStr = "网络错误(无网络/防火墙拦截)";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorStr = "连接超时(网络延迟/服务端无响应)";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorStr = "远端主机关闭连接";
            break;
        default:
            errorStr = QString("未知错误(错误码：%1)").arg((int)socketError);
            break;
    }
    BSD_LOG_INFO(QString("Socket连接失败: ID = %1, 错误类型 = %2(错误码：%3)\n")
                 .arg(id).arg(errorStr).arg((int)socketError));

}

void GetImgViewLeft::videoStatusUpdate(int id)
{
    SYSTEM_STATUS_E *videoStatus = nullptr;
    QPushButton *targetBtn = nullptr;
    if(id == 1)
    {
        videoStatus = &videoStatus1;
        targetBtn = ui->btnOpen1;
    }
    else
    {
        videoStatus = &videoStatus2;
        targetBtn = ui->btnOpen2;
    }
    *videoStatus = (*videoStatus == SYSTEM_STATUS_OPEN) ? SYSTEM_STATUS_CLOSE : SYSTEM_STATUS_OPEN;
    if(*videoStatus == SYSTEM_STATUS_OPEN)
    {
        targetBtn->setText("close");
    }else
    {
        targetBtn->setText("open");
    }
    BSD_LOG_INFO(QString("ip%1 %2成功\n").arg(id).arg((*videoStatus == SYSTEM_STATUS_OPEN )? "打开" : "关闭"));
}
void GetImgViewLeft::sendDataSuccessSlot(int id, GetImgAppCmd cmd)
{
    switch(cmd.getCmd())
    {
        case GET_IMG_CMD_VIDEO:
        {
            videoStatusUpdate(id);
        }break;
        case GET_IMG_CMD_LED:
        {
            emit socketUpdateLedStatus(id, cmd.getFlag() == GET_IMG_CMD_OPEN ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
        }break;
        case GET_IMG_CMD_IRCUT:
        {
            emit socketUpdateIrcutStatus(id, cmd.getFlag() == GET_IMG_CMD_OPEN ? SYSTEM_STATUS_OPEN : SYSTEM_STATUS_CLOSE);
        }break;
        default:
            break;
    }
}
void GetImgViewLeft::showImgSlot(IMG_SRC_TYPE_E imgSrcType, QImage img)
{
    if (img.isNull()) return;
    static int showCount = 0;     
    QLabel* targetLabel = nullptr;
    if(imgSrcType == IMG_SRC_IR)
        targetLabel = ui->labelIR;
    else if(imgSrcType == IMG_SRC_RGB)
        targetLabel = ui->labelRGB;
    else
        return;
    targetLabel->setPixmap(QPixmap::fromImage(img));
    if (++showCount >= 15) {
        showCount = 0;           
        targetLabel->repaint();  
    }
}
void GetImgViewLeft::on_btnConnect1_clicked()
{
    int ret = 0;
    SOCKET_CONNECT_STATUS_E status = getImgAppSocket1->getConnected();
    if(status == SOCKET_DISCONNECT)
    {
        ret = getImgAppSocket1->connectToSrever(ui->lineEditIP1->text(), ui->lineEditProt1->text());
        if(ret < 0)
        {
            BSD_LOG_INFO("连接ip1失败\n");
        }
    }
    else if(status == SOCKET_CONNECTED)
    {
        BSD_LOG_INFO("正在连接中，请勿重复点击~\n");
    }else if(status == SOCKET_CONNECT)
    {
        ret = getImgAppSocket1->disconnectToServer();
        if(ret < 0)
        {
            BSD_LOG_INFO("断开ip1连接失败\n");
        }
    }
}


void GetImgViewLeft::on_btnConnect2_clicked()
{
    int ret = 0;
    SOCKET_CONNECT_STATUS_E status = getImgAppSocket2->getConnected();
    if(status == SOCKET_DISCONNECT)
    {
        ret = getImgAppSocket2->connectToSrever(ui->lineEditIP2->text(), ui->lineEditProt2->text());
        if(ret < 0)
        {
            BSD_LOG_INFO("连接ip2失败\n");
        }
    }
    else if(status == SOCKET_CONNECTED)
    {
        BSD_LOG_INFO("正在连接中，请勿重复点击~\n");
    }else if(status == SOCKET_CONNECT)
    {
        ret = getImgAppSocket2->disconnectToServer();
        if(ret < 0)
        {
            BSD_LOG_INFO("断开ip2连接失败\n");
        }
    }
}


void GetImgViewLeft::on_btnOpen1_clicked()
{
    GetImgAppCmd cmd;
    cmd.setCmd(GET_IMG_CMD_VIDEO);
    cmd.setFlag((videoStatus1 == SYSTEM_STATUS_OPEN) ? GET_IMG_CMD_CLOSE : GET_IMG_CMD_OPEN);
    getImgAppSocket1->sendDataToServer(cmd);    
}


void GetImgViewLeft::on_btnOpen2_clicked()
{
    GetImgAppCmd cmd;
    cmd.setCmd(GET_IMG_CMD_VIDEO);
    cmd.setFlag((videoStatus2 == SYSTEM_STATUS_OPEN) ? GET_IMG_CMD_CLOSE : GET_IMG_CMD_OPEN);
    getImgAppSocket2->sendDataToServer(cmd);
}

