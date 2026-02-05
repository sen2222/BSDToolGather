#include "getimgapp.h"
#include "alltoolfun.h"
GetImgAppSocket::GetImgAppSocket(int id, RingBuffer *ringBuffer, QObject *parent)
    : QObject(parent), socketId(id), ringBuffer(ringBuffer)
{
    connectStatus = 0;
    workThread = new QThread();
    connect(this, &GetImgAppSocket::newTcpSocketSignal, this, &GetImgAppSocket::newTcpSocketSlot, Qt::QueuedConnection);
    connect(this, &GetImgAppSocket::deleteTcpSocketSignal, this, &GetImgAppSocket::deleteTcpSocketSlot, Qt::QueuedConnection);
    connect(this, &GetImgAppSocket::connectToServerSignal, this, &GetImgAppSocket::connecteToServerSlot, Qt::QueuedConnection);
    connect(this, &GetImgAppSocket::disconnectToServerSignal, this, &GetImgAppSocket::disconnectToServerSlot, Qt::QueuedConnection);
    connect(this, &GetImgAppSocket::sendDataToServerSignal, this, &GetImgAppSocket::sendDataToServerSlot, Qt::QueuedConnection);

    this->moveToThread(workThread);
    workThread->start();
    emit newTcpSocketSignal();
}

GetImgAppSocket::~GetImgAppSocket() 
{
    emit deleteTcpSocketSignal();
    if (workThread && workThread->isRunning()) {
        workThread->quit();
        if (!workThread->wait(3000)) {
            workThread->terminate();
            workThread->wait();
        }
        delete workThread;
        workThread = nullptr;
    }
}


int GetImgAppSocket::getConnected()
{
    QMutexLocker locker(&socketMutex);
    return connectStatus;
}

int GetImgAppSocket::connectToSrever(const QString &ip, const QString &port)
{
    int ret = 0;
    int protInt;
    if(ip.isEmpty() || port.isEmpty())
    {
        BSD_LOG(LOG_ERRO, "ip is empty or port is empty\n");
        return -1;
    }
    if(getConnected() != 0)
    {
        BSD_LOG(LOG_ERRO, QString("socket is connected, status = %1\n").arg(connectStatus));
        return -1;
    }
    ret = isIpValid(ip);
    if(!ret)
    {
        BSD_LOG(LOG_ERRO, QString("Invalid IP address: %1\n").arg(ip));
        return -1;
    }
    protInt = convertPortToInt(port);
    if(protInt < 0)
    {
        BSD_LOG(LOG_ERRO, QString("Invalid port number: %1\n").arg(port));
        return -1;
    }
    emit connectToServerSignal(ip, protInt);
    return 0;
}


int GetImgAppSocket::disconnectToServer()
{
    if(getConnected() != 2)
    {
        BSD_LOG(LOG_ERRO, "socket is not connected\n");
        return -1;
    }
    emit disconnectToServerSignal();
    return 0;
}
int GetImgAppSocket::sendDataToServer(const QByteArray &data)
{
    if(getConnected() != 2)
    {
        BSD_LOG(LOG_ERRO, "socket is not connected\n");
        return -1;
    }
    emit sendDataToServerSignal(data);
    return 0;
}



bool GetImgAppSocket::isIpValid(const QString &ip)
{
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return ipRegex.match(ip).hasMatch();
}

int GetImgAppSocket::convertPortToInt(const QString &port)
{
    bool ok;
    int portInt = port.toInt(&ok);
    if (!ok || portInt < 0 || portInt > 65535) {
        BSD_LOG(LOG_ERRO, QString("Invalid port number: %1").arg(port));
        return -1;
    }
    return portInt;
}

void GetImgAppSocket::newTcpSocketSlot()
{
    QMutexLocker locker(&socketMutex);
    if(tcpSocket)
    {
       return;
    }
    tcpSocket = new QTcpSocket();

    // 1. 连接成功回调：传递 Socket 标识
    connect(tcpSocket, &QTcpSocket::connected, this, [this]() {
        QMutexLocker locker(&socketMutex);
        connectStatus = 2;
        emit connectSuccessSignal(socketId);
    }, Qt::QueuedConnection);

    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        QMutexLocker locker(&socketMutex);
        connectStatus = 0;
        emit socketErrorSignal(socketId, error);
    }, Qt::QueuedConnection);
    connect(tcpSocket, &QTcpSocket::disconnected, this, [this]() {
        // QMutexLocker locker(&socketMutex);
        connectStatus = 0;
        emit disconnectSuccessSignal(socketId);
    }, Qt::QueuedConnection);
    connect(tcpSocket, &QTcpSocket::bytesWritten, this, [this](qint64 bytes) {
        Q_UNUSED(bytes);
        emit sendDataSuccessSignal(socketId);
    }, Qt::QueuedConnection);
    
    connect(tcpSocket, &QTcpSocket::readyRead, this, &GetImgAppSocket::recvDataFromServerSlot, Qt::QueuedConnection);
}

void GetImgAppSocket::deleteTcpSocketSlot()
{

    QMutexLocker locker(&socketMutex);
    if(tcpSocket)
    {
        tcpSocket->abort();
        delete tcpSocket;
        tcpSocket = nullptr;
        connectStatus = 0;
    }
}



void GetImgAppSocket::connecteToServerSlot(QString ip, int port)
{
    BSD_LOG_INFO(QString("ip%1连接服务器中: %2, %3\n").arg(socketId).arg(ip).arg(port));
    QMutexLocker locker(&socketMutex);
    connectStatus = 1;
    tcpSocket->connectToHost(ip, port);
}
void GetImgAppSocket::disconnectToServerSlot()
{
    QMutexLocker locker(&socketMutex);
    BSD_LOG_INFO(QString("ip%1断开服务器连接\n").arg(socketId));
    tcpSocket->disconnectFromHost();
}
void GetImgAppSocket::sendDataToServerSlot(QByteArray data)
{
    QMutexLocker locker(&socketMutex);
    if(tcpSocket)
    {
        tcpSocket->write(data);
    }
}

void GetImgAppSocket::recvDataFromServerSlot()
{
    uint32_t recvSize = 0;
    QMutexLocker locker(&socketMutex);
    if(!tcpSocket || connectStatus != 2 || tcpSocket->bytesAvailable() <= 0 || tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }
    QByteArray data = tcpSocket->readAll();
    if(data.isEmpty())
    {
        return;
    }
    if(ringBuffer)
    {
        recvSize = ringBuffer->writeWait(reinterpret_cast<uint8_t*>(data.data()), data.size(), 1000);
        if(recvSize != data.size())
        {
            QThread::msleep(500);
        }
    }else{
        QThread::msleep(500);
    }
}



/**********GetImgAppCmd**********/
GetImgAppCmd::GetImgAppCmd()
{
    memset(&cmdInfo, 0, sizeof(GET_IMG_CMD_S));
    cmdInfo.headH = GET_IMG_CMD_HEAD_H;
    cmdInfo.headL = GET_IMG_CMD_HEAD_L;
}
GetImgAppCmd::~GetImgAppCmd()
{
    
}

void GetImgAppCmd::setCmd(uint8_t cmd)
{
    cmdInfo.cmd = cmd;
}
void GetImgAppCmd::clearAll()
{
    uint8_t *data = (uint8_t *)&cmdInfo.cmd;
    memset(data, 0, sizeof(GET_IMG_CMD_S) - 2);
}
QByteArray GetImgAppCmd::toByteArray()
{
    return QByteArray((const char *)&cmdInfo, sizeof(GET_IMG_CMD_S));
}



/**********getImgAppAnalysisFrame**********/
GetImgAppAnalysisFrame::GetImgAppAnalysisFrame(RingBuffer *ringBuffer, QObject *parent)
    : QThread(parent), ringBuffer(ringBuffer)
{
    analysisState = State_Pause;
    this->start();
    
}
GetImgAppAnalysisFrame::~GetImgAppAnalysisFrame()
{
    stopAnalysis();
    wait();
}


void GetImgAppAnalysisFrame::pauseAnalysis()
{
    QMutexLocker locker(&analysisStateMutex);
    if(analysisState == State_Running)
    {
        analysisState = State_Pause;
        analysisStateWaitCondition.wakeAll();
    }
}
void GetImgAppAnalysisFrame::startAnalysis()
{
    QMutexLocker locker(&analysisStateMutex);
    if(analysisState == State_Stop || analysisState == State_Pause)
    {
        analysisState = State_Running;
        analysisStateWaitCondition.wakeAll();
    }
}
void GetImgAppAnalysisFrame::stopAnalysis()
{
    QMutexLocker locker(&analysisStateMutex);
    if(analysisState == State_Running)
    {
        analysisState = State_Stop;
        analysisStateWaitCondition.wakeAll();
    }
}



void GetImgAppAnalysisFrame::run()
{
    int ret = 0;
    uint32_t recvSize = 0;
    uint8_t *imgData = nullptr;
    RECV_IMG_INFO_S imgInfo;
    QImage showImg;
    while(1)
    {
        {
            QMutexLocker locker(&analysisStateMutex);
            while (analysisState == State_Pause)
                analysisStateWaitCondition.wait(&analysisStateMutex);
            if (analysisState == State_Stop)
                break;
        }

        memset(&imgInfo, 0, sizeof(imgInfo));
        // 先接收图片信息
        recvSize = ringBuffer->readWait((uint8_t *)&imgInfo, sizeof(imgInfo), 1000);
        if(recvSize != sizeof(imgInfo) || imgInfo.headH != GET_IMG_CMD_HEAD_H || imgInfo.headL != GET_IMG_CMD_HEAD_L)
        {
            BSD_LOG(LOG_WARN, QString("read img info failed, recvSize = %1, size = %2\n").arg(recvSize).arg(sizeof(imgInfo)));
            QThread::msleep(500);
            continue;
        }
        imgData = new uint8_t[imgInfo.imgSize];
        recvSize = ringBuffer->readWait(imgData, imgInfo.imgSize, 1000);
        if(recvSize != imgInfo.imgSize)
        {
            BSD_LOG(LOG_WARN, QString("read img data failed, recvSize = %1, size = %2\n").arg(recvSize).arg(imgInfo.imgSize));
            goto RECV_IMG_ERRO;
        }
        BSD_LOG_INFO(QString("recv img, size = %1\n").arg(imgInfo.imgSize));
        // 解码图片
        ret = imgDecoder.yuv420decode(imgData, imgInfo.width, imgInfo.height, showImg);
        if(ret < 0)
        {
            BSD_LOG(LOG_WARN, QString("yuv420decode failed, ret = %1\n").arg(ret));
            goto RECV_IMG_ERRO;
        }
        // 发送图片到界面
        emit showImgSignal(imgInfo.imgSrcType, showImg);
        BSD_LOG_INFO(QString("show img, width = %1, height = %2\n").arg(showImg.width()).arg(showImg.height()));
        
RECV_IMG_ERRO:
        delete[] imgData;
        imgData = nullptr;
    }
}




