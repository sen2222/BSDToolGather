#include "getimgviewleft.h"
#include "ui_getimgviewleft.h"

#include "alltoolfun.h"

GetImgViewLeft::GetImgViewLeft(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GetImgViewLeft)
{
    ui->setupUi(this);

    ip1RingBuffer = new RingBuffer(IP1_RING_BUFFER_SIZE);
    ip2RingBuffer = new RingBuffer(IP2_RING_BUFFER_SIZE);

    videoStatus1 = 0;
    videoStatus2 = 0;
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
    
    connect(getImgAppSocket2, &GetImgAppSocket::connectSuccessSignal, this, &GetImgViewLeft::connectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::disconnectSuccessSignal, this, &GetImgViewLeft::disconnectSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::socketErrorSignal, this, &GetImgViewLeft::socketErrorSlot, Qt::QueuedConnection);
    connect(getImgAppSocket2, &GetImgAppSocket::sendDataSuccessSignal, this, &GetImgViewLeft::sendDataSuccessSlot, Qt::QueuedConnection);
    connect(getImgAppAnalysisFrame2, &GetImgAppAnalysisFrame::showImgSignal, this, &GetImgViewLeft::showImgSlot, Qt::QueuedConnection);
}


GetImgViewLeft::~GetImgViewLeft()
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
    delete ui;
}


void GetImgViewLeft::connectSuccessSlot(int id)
{
    if(id == 1)
    {
        ui->btnConnect1->setText("disconnect");
        ui->btnOpen1->setText("open");
        ui->btnOpen1->setEnabled(true);
    }
    else if(id == 2)
    {
        ui->btnConnect2->setText("disconnect");
        ui->btnOpen2->setText("open");
        ui->btnOpen2->setEnabled(true);
    }
    BSD_LOG_INFO(QString("Socket连接成功: ID = %1\n").arg(id));
}
void GetImgViewLeft::disconnectSuccessSlot(int id)
{

    if(id == 1)
    {
        ui->btnConnect1->setText("connect");
        ui->btnOpen1->setText("");
        ui->btnOpen1->setEnabled(false);
        videoStatus1 = 0;
        getImgAppAnalysisFrame1->pauseAnalysis();
    }
    else if(id == 2)
    {
        ui->btnConnect2->setText("connect");
        ui->btnOpen2->setText("");
        ui->btnOpen2->setEnabled(false);
        videoStatus2 = 0;
        getImgAppAnalysisFrame2->pauseAnalysis();
    }
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
void GetImgViewLeft::sendDataSuccessSlot(int id)
{
    int *videoStatus = nullptr;
    QPushButton *targetBtn = nullptr;
    GetImgAppAnalysisFrame *targetAnalysisFrame = nullptr;
    if(id == 1)
    {
        videoStatus = &videoStatus1;
        targetBtn = ui->btnOpen1;
        targetAnalysisFrame = getImgAppAnalysisFrame1;
    }
    else
    {
        videoStatus = &videoStatus2;
        targetBtn = ui->btnOpen2;
        targetAnalysisFrame = getImgAppAnalysisFrame2;
    }
    *videoStatus = !(*videoStatus);
    if(*videoStatus)
    {
        targetBtn->setText("close");
        targetAnalysisFrame->startAnalysis();
    }else
    {
        targetBtn->setText("open");
        targetAnalysisFrame->pauseAnalysis();
    }
    BSD_LOG_INFO(QString("ip%1 %2成功\n").arg(id).arg(*videoStatus ? "打开" : "关闭"));
}

void GetImgViewLeft::showImgSlot(IMG_SRC_TYPE_E imgSrcType, QImage img)
{
    BSD_LOG_INFO(QString("showImgSlot: imgSrcType = %1, imgSize = %2\n").arg(imgSrcType).arg(img.sizeInBytes()));
    if (img.isNull()) return;

    QLabel* targetLabel = nullptr;
    if(imgSrcType == IMG_SRC_IR)
    {
        targetLabel = ui->labelIR;
    }
    else if(imgSrcType == IMG_SRC_RGB)
    {
        targetLabel = ui->labelRGB;
    }
    if (!targetLabel) return;

    QSize labSize = targetLabel->size();
    if (labSize.isEmpty()) return;

    // 核心：先裁剪至Label比例，再缩放填充
    qreal labRatio = (qreal)labSize.width() / labSize.height();
    qreal imgRatio = (qreal)img.width() / img.height();
    QImage imgCrop = img;

    if (qAbs(imgRatio - labRatio) > 1e-6)
    {
        int cropW = img.width();
        int cropH = img.height();
        imgRatio > labRatio ? cropW = img.height() * labRatio : cropH = img.width() / labRatio;
        int x = (img.width() - cropW) / 2;
        int y = (img.height() - cropH) / 2;
        imgCrop = img.copy(x, y, cropW, cropH);
    }

    QPixmap pix = QPixmap::fromImage(imgCrop);
    pix = pix.scaled(labSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    targetLabel->setScaledContents(false);
    targetLabel->setPixmap(pix);
}



void GetImgViewLeft::on_btnConnect1_clicked()
{
    int ret = 0;
    int status = getImgAppSocket1->getConnected();
    if(status == 0)
    {
        ret = getImgAppSocket1->connectToSrever(ui->lineEditIP1->text(), ui->lineEditProt1->text());
        if(ret < 0)
        {
            BSD_LOG_INFO("连接ip1失败\n");
        }
    }
    else if(status == 1)
    {
        BSD_LOG_INFO("正在连接中，请勿重复点击~\n");
    }else if(status == 2)
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
    int status = getImgAppSocket2->getConnected();
    if(status == 0)
    {
        ret = getImgAppSocket2->connectToSrever(ui->lineEditIP2->text(), ui->lineEditProt2->text());
        if(ret < 0)
        {
            BSD_LOG_INFO("连接ip2失败\n");
        }
    }
    else if(status == 1)
    {
        BSD_LOG_INFO("正在连接中，请勿重复点击~\n");
    }else if(status == 2)
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
    int cmdTmp = videoStatus1 ? GET_IMG_CMD_CLOSE : GET_IMG_CMD_OPEN;
    GetImgAppCmd cmd;
    cmd.setCmd(cmdTmp);
    getImgAppSocket1->sendDataToServer(cmd.toByteArray());
    
}


void GetImgViewLeft::on_btnOpen2_clicked()
{
    int cmdTmp = videoStatus2 ? GET_IMG_CMD_CLOSE : GET_IMG_CMD_OPEN;
    GetImgAppCmd cmd;
    cmd.setCmd(cmdTmp);
    getImgAppSocket2->sendDataToServer(cmd.toByteArray());
}

