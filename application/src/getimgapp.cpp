#include "getimgapp.h"
#include "alltoolfun.h"




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
void GetImgAppCmd::setFlag(uint8_t flag)
{
    cmdInfo.flag = flag;
}
uint8_t GetImgAppCmd::getCmd()
{
    return cmdInfo.cmd;
}
uint8_t GetImgAppCmd::getFlag()
{
    return cmdInfo.flag;
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

/**********GetImgAppSocket**********/
GetImgAppSocket::GetImgAppSocket(int id, RingBuffer *ringBuffer, QObject *parent)
    : QObject(parent), socketId(id), ringBuffer(ringBuffer)
{
    connectStatus = SOCKET_DISCONNECT;
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
SOCKET_CONNECT_STATUS_E GetImgAppSocket::getConnected()
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
int GetImgAppSocket::sendDataToServer(GetImgAppCmd &cmd)
{
    if(getConnected() != 2)
    {
        BSD_LOG(LOG_ERRO, "socket is not connected\n");
        return -1;
    }
    emit sendDataToServerSignal(cmd);
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
        connectStatus = SOCKET_CONNECT;
        emit connectSuccessSignal(socketId);
    }, Qt::QueuedConnection);

    connect(tcpSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        QMutexLocker locker(&socketMutex);
        connectStatus = SOCKET_DISCONNECT;
        emit socketErrorSignal(socketId, error);
    }, Qt::QueuedConnection);
    connect(tcpSocket, &QTcpSocket::disconnected, this, [this]() {
        // QMutexLocker locker(&socketMutex);
        connectStatus = SOCKET_DISCONNECT;
        emit disconnectSuccessSignal(socketId);
    }, Qt::QueuedConnection);
    connect(tcpSocket, &QTcpSocket::bytesWritten, this, [this](qint64 bytes) {
        Q_UNUSED(bytes);
        QMutexLocker lock(&cmdMutex);
        GetImgAppCmd cmd = cmdQueue.dequeue();
        emit sendDataSuccessSignal(socketId, cmd);
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
        connectStatus = SOCKET_DISCONNECT;
    }
}

void GetImgAppSocket::connecteToServerSlot(QString ip, int port)
{
    BSD_LOG_INFO(QString("ip%1连接服务器中: %2, %3\n").arg(socketId).arg(ip).arg(port));
    QMutexLocker locker(&socketMutex);
    connectStatus = SOCKET_CONNECTED;
    tcpSocket->connectToHost(ip, port);
}
void GetImgAppSocket::disconnectToServerSlot()
{
    QMutexLocker locker(&socketMutex);
    BSD_LOG_INFO(QString("ip%1断开服务器连接\n").arg(socketId));
    tcpSocket->disconnectFromHost();
}
void GetImgAppSocket::sendDataToServerSlot(GetImgAppCmd cmd)
{
    QMutexLocker locker(&socketMutex);
    if(tcpSocket)
    {
        tcpSocket->write(cmd.toByteArray());
    }
    QMutexLocker lock(&cmdMutex);
    cmdQueue.enqueue(cmd);
}

void GetImgAppSocket::recvDataFromServerSlot()
{
    QMutexLocker locker(&socketMutex);
    if(!tcpSocket || connectStatus != SOCKET_CONNECT || tcpSocket->bytesAvailable() <= 0 || tcpSocket->state() != QAbstractSocket::ConnectedState)
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
        ringBuffer->writeWait(reinterpret_cast<uint8_t*>(data.data()), data.size(), 3000);
        emit analysisDataUpdateSignal();
    }
   
}


/**********getImgAppSaveFrame**********/
#include "GetImgViewRight.h" 
GetImgAppSaveFrame::GetImgAppSaveFrame(SAVE_THREAD_SAVE_PATH_S &savePath, QObject *parent)
    : QThread(parent), savePath(savePath)
{
    saveState = SAVE_THREAD_STATE_GET_INFO;
    this->start();
    if(!savePath.rightView)
        stopSave();
}

GetImgAppSaveFrame::~GetImgAppSaveFrame()
{
    stopSave();
    wait();
}

void GetImgAppSaveFrame::stopSave()
{
    QMutexLocker locker(&saveMutex);
    saveState = SAVE_THREAD_STATE_STOP;
    saveWaitCondition.wakeAll();
}
SAVE_THREAD_STATE_E GetImgAppSaveFrame::getSaveState()
{
    QMutexLocker locker(&saveMutex);
    return saveState;
}
void GetImgAppSaveFrame::addFrameToSave(RECV_IMG_INFO_S &imgInfo, uint8_t *data, QImage &image, SAVE_THREAD_TYPE_E saveType)
{
    if (!data || getSaveState() == SAVE_THREAD_STATE_STOP)
        return;

    SAVE_THREAD_DATA_S *frame = new SAVE_THREAD_DATA_S;
    frame->imgInfo = imgInfo;
    frame->yuvData = QByteArray((const char*)data, imgInfo.imgSize);
    frame->imgData = image;
    frame->saveType = saveType;
    QMutexLocker lock(&saveMutex);
    saveQueue.enqueue(frame);
    saveWaitCondition.wakeAll();
}
void GetImgAppSaveFrame::clearSaveQueue()
{
    SAVE_THREAD_DATA_S *frame = nullptr;
    QMutexLocker lock(&saveMutex);
    while(!saveQueue.isEmpty())
    {
        frame = saveQueue.dequeue();
        delete frame;
        frame = nullptr;
    }
}
void GetImgAppSaveFrame::packCurrentSaveDir()
{
    QMutexLocker locker(&saveMutex);
    QString timeStr = QDateTime::currentDateTime().toString("MMdd_hhmmss");
    QStringList processed;
    bool hasPacked = false;

    if (!processed.contains(savePath.IRSavePath)) {
        QDir dir(savePath.IRSavePath);
        if (dir.exists() && !dir.isEmpty()) {
            QString parent = dir.absoluteFilePath("..");
            QString newName;

            if (savePath.IRSavePath == savePath.RGBSavePath) {
                newName = QString("image-YUV-%1").arg(timeStr);
            } else {
                newName = QString("image-YUV-IR-%1").arg(timeStr);
            }

            QString newPath = QDir(parent).filePath(newName);
            dir.rename(savePath.IRSavePath, newPath);
            hasPacked = true;
        }
        QDir().mkpath(savePath.IRSavePath);
        processed << savePath.IRSavePath;
    }
    if (!processed.contains(savePath.RGBSavePath)) {
        QDir dir(savePath.RGBSavePath);

        if (dir.exists() && !dir.isEmpty()) {
            QString parent = dir.absoluteFilePath("..");
            QString newName = QString("image-YUV-RGB-%1").arg(timeStr);
            QString newPath = QDir(parent).filePath(newName);

            dir.rename(savePath.RGBSavePath, newPath);
            hasPacked = true;
        }
        QDir().mkpath(savePath.RGBSavePath);
        processed << savePath.RGBSavePath;
    }

    if (!processed.contains(savePath.BMPSavePath)) {
        QDir dir(savePath.BMPSavePath);
        if (dir.exists() && !dir.isEmpty()) {
            QString parent = dir.absoluteFilePath("..");
            QString newName = QString("image-BMP-%1").arg(timeStr);
            QString newPath = QDir(parent).filePath(newName);
            dir.rename(savePath.BMPSavePath, newPath);
            hasPacked = true;
        }
        processed << savePath.BMPSavePath;
    }
    if (!processed.contains(savePath.JPEGSavePath)) {
        QDir dir(savePath.JPEGSavePath);
        if (dir.exists() && !dir.isEmpty()) {
            QString parent = dir.absoluteFilePath("..");
            QString newName = QString("image-JPEG-%1").arg(timeStr);
            QString newPath = QDir(parent).filePath(newName);
            dir.rename(savePath.JPEGSavePath, newPath);
            hasPacked = true;
        }
        processed << savePath.JPEGSavePath;
    }
    if (!processed.contains(savePath.PNGSavePath)) {
        QDir dir(savePath.PNGSavePath);
        if (dir.exists() && !dir.isEmpty()) {
            QString parent = dir.absoluteFilePath("..");
            QString newName = QString("image-PNG-%1").arg(timeStr);
            QString newPath = QDir(parent).filePath(newName);
            dir.rename(savePath.PNGSavePath, newPath);
            hasPacked = true;
        }
        // 这里不执行 mkpath，不会自动创建
        processed << savePath.PNGSavePath;
    }

    if (hasPacked) {
        BSD_LOG_INFO("------- IR/RGB/BMP/PNG 目录打包完成 ------\n");
    }
    saveWaitCondition.wakeAll();
}
QString GetImgAppSaveFrame::getSaveTypeChinese(SAVE_THREAD_TYPE_E saveType)
{
    QString saveTypeStr = "";
    switch(saveType)
    {
        case SAVE_IMAGE_NOT:
            return "不保存";
        case SAVE_IMAGE_BMP:
            return "BMP";
        case SAVE_IMAGE_JPEG:
            return "JPEG";
        case SAVE_IMAGE_PNG:
            return "PNG";
        default:
            return "未知";
    }
}

int GetImgAppSaveFrame::WriteBmpFromARGB(const QString& path, const QImage& img)
{
    if (img.isNull() || img.format() != QImage::Format_ARGB32)
        return false;

    int w = img.width();
    int h = img.height();
    qint64 dataSize = img.sizeInBytes();

#pragma pack(push, 1)
    struct BmpFileHeader {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };

    struct BmpInfoHeader {
        uint32_t biSize;
        int32_t  biWidth;
        int32_t  biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t  biXPelsPerMeter;
        int32_t  biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
#pragma pack(pop)
    BmpFileHeader fileHeader;
    fileHeader.bfType = 0x4D42;         // "BM" 标识
    fileHeader.bfSize = 54 + dataSize;  // 文件总大小 = 头(54) + 像素数据
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = 54;          // 像素数据偏移

    BmpInfoHeader infoHeader;
    infoHeader.biSize = 40;             // 信息头大小
    infoHeader.biWidth = w;             // 图像宽度
    infoHeader.biHeight = -h;           // 负数表示正向存储，不翻转
    infoHeader.biPlanes = 1;            // 固定为1
    infoHeader.biBitCount = 32;         // 32位ARGB
    infoHeader.biCompression = 0;       // 无压缩
    infoHeader.biSizeImage = dataSize;  // 像素数据大小
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return -1;

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    file.write(reinterpret_cast<const char*>(img.constBits()), dataSize);

    return 0;
}
int GetImgAppSaveFrame::saveFrameToFile(SAVE_THREAD_DATA_S *frame)
{
    if (!frame || frame->yuvData.isEmpty() || !savePath.rightView) {
        return -1;
    }

    QDir dir;
    RECV_IMG_INFO_S info = frame->imgInfo;
    QString typeStr = (info.imgSrcType == IMG_SRC_IR) ? "IR" : "RGB";
    uint32_t saveNum = savePath.rightView->getImgSaveNum(info.imgSrcType);
    QString saveFileName = QString("image_%1_%2_%3x%4")
                           .arg(saveNum)
                           .arg(typeStr)
                           .arg(info.width)
                           .arg(info.height);

    QString saveImgPath = (info.imgSrcType == IMG_SRC_IR) ? savePath.IRSavePath : savePath.RGBSavePath;
    if (!dir.exists(saveImgPath)) {
        dir.mkpath(saveImgPath);
    }

    QString yuvPath = saveImgPath + "/" + saveFileName + ".yuv";
    QFile file(yuvPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return -1;
    }
    file.write(frame->yuvData);
    file.close();
    switch(frame->saveType)
    {
        case SAVE_IMAGE_NOT:
            break;
        case SAVE_IMAGE_BMP:
        {
            if (!dir.exists(savePath.BMPSavePath)) {
                dir.mkpath(savePath.BMPSavePath);
            }
            QString bmpPath = savePath.BMPSavePath + "/" + saveFileName + ".bmp";
            if (!frame->imgData.isNull()) {
                WriteBmpFromARGB(bmpPath, frame->imgData);
                // frame->imgData.save(bmpPath, "BMP");
            }
        } break;
        case SAVE_IMAGE_JPEG:
        {
            if (!dir.exists(savePath.PNGSavePath)) {
                dir.mkpath(savePath.JPEGSavePath);
            }
            if (!frame->imgData.isNull()) {
                frame->imgData.save(savePath.JPEGSavePath + "/" + saveFileName + ".jpg", "JPEG", 80);
            }
        }break;
        case SAVE_IMAGE_PNG:
        {
            if (!dir.exists(savePath.PNGSavePath)) {
                dir.mkpath(savePath.PNGSavePath);
            }
            if (!frame->imgData.isNull()) {
                frame->imgData.save(savePath.PNGSavePath + "/" + saveFileName + ".png", "PNG");
            }
        }break;
        default:
            break;
    }
    savePath.rightView->addImgSaveNum(info.imgSrcType);
    return 0;
}

void GetImgAppSaveFrame::run()
{
    int ret = 0;
    SAVE_THREAD_DATA_S *frame = nullptr;
    while(1)
    {
        if (getSaveState() == SAVE_THREAD_STATE_STOP)
            break;

        saveMutex.lock();
        saveWaitCondition.wait(&saveMutex);

        if(saveQueue.isEmpty())
            goto SAVE_FRAME_THREAD_FREE;
        frame = saveQueue.dequeue();
        ret = saveFrameToFile(frame);
        if(ret < 0)
        {
            BSD_LOG(LOG_WARN, QString("saveFrameToFile failed, ret = %1\n").arg(ret));
            goto SAVE_FRAME_THREAD_FREE;
        }
        savePath.rightView->addImgSaveNum(frame->imgInfo.imgSrcType);

SAVE_FRAME_THREAD_FREE:
        if(frame)
        {
            delete frame;
            frame = nullptr;
        }
        saveMutex.unlock();
    }
    // 清理队列
    clearSaveQueue();
}

/**********getImgAppAnalysisFrame**********/
GetImgAppAnalysisFrame::GetImgAppAnalysisFrame(RingBuffer *ringBuffer, QObject *parent)
    : QThread(parent), ringBuffer(ringBuffer)
{
    analysisState = ANALYSIS_THREAD_STATE_GET_HANDLE;
    saveFlag = SYSTEM_STATUS_CLOSE;
    this->start();
    setAngle(I2D_ANGLE_0);
    setSaveEncoderType(SAVE_IMAGE_NOT);
}
GetImgAppAnalysisFrame::~GetImgAppAnalysisFrame()
{
    stopAnalysis();
    wait();
}


void GetImgAppAnalysisFrame::stopAnalysis()
{
    QMutexLocker locker(&analysisDataMutex);
    analysisState = ANALYSIS_THREAD_STATE_STOP;
    analysisDataWaitCondition.wakeAll();
}
void GetImgAppAnalysisFrame::analysisDataUpdateSlot()
{
    QMutexLocker locker(&analysisDataMutex);
    analysisDataWaitCondition.wakeAll();
}

void GetImgAppAnalysisFrame::setAngle(I2D_ANGLE_E angle)
{
    QMutexLocker locker(&angleMutex);
    this->angle = angle;
    // BSD_LOG_INFO(QString("显示旋转角度: %1\n").arg(angle));
}
I2D_ANGLE_E GetImgAppAnalysisFrame::getAngle()
{
    QMutexLocker locker(&angleMutex);
    return angle;
}
void GetImgAppAnalysisFrame::setSaveFlag(SYSTEM_STATUS_E flag)
{
    QMutexLocker locker(&saveFlagMutex);
    saveFlag = flag;
}
SYSTEM_STATUS_E GetImgAppAnalysisFrame::getSaveFlag()
{
    QMutexLocker locker(&saveFlagMutex);
    return saveFlag;
}
SAVE_THREAD_TYPE_E GetImgAppAnalysisFrame::getSaveEncoderType()
{
    QMutexLocker locker(&saveEncoderTypeMutex);
    return saveEncoderType;
}
void GetImgAppAnalysisFrame::setSaveEncoderType(SAVE_THREAD_TYPE_E saveEncoderType)
{
    QMutexLocker locker(&saveEncoderTypeMutex);
    this->saveEncoderType = saveEncoderType;
}
void GetImgAppAnalysisFrame::startSaveFrame(GetImgAppSaveFrame *SaveFrame)
{
    if(SaveFrame == nullptr)
        return;
    this->SaveFrame = SaveFrame;
    setSaveFlag(SYSTEM_STATUS_OPEN);
}
void GetImgAppAnalysisFrame::stopSaveFrame()
{
    setSaveFlag(SYSTEM_STATUS_CLOSE);
    this->SaveFrame = nullptr;
}

void GetImgAppAnalysisFrame::setShowAngle(int angle)
{
    switch(angle)
    {
        case 0:
            setAngle(I2D_ANGLE_0);
            break;
        case 1:
            setAngle(I2D_ANGLE_90);
            break;
        case 2:
            setAngle(I2D_ANGLE_180);
            break;
        case 3:
            setAngle(I2D_ANGLE_270);
            break;
        default:
            break;
    }
}
void GetImgAppAnalysisFrame::setSaveType(int index)
{
    switch(index)
    {
        case 0:
            setSaveEncoderType(SAVE_IMAGE_NOT);
            break;
        case 1:
            setSaveEncoderType(SAVE_IMAGE_BMP);
            break;
        case 2:
            setSaveEncoderType(SAVE_IMAGE_JPEG);
            break;
        case 3:
            setSaveEncoderType(SAVE_IMAGE_PNG);
            break;
        default:
            break;
    }
}
QString GetImgAppAnalysisFrame::getSaveEncoderChinese(int index)
{
    switch(index)
    {
        case 0:
            return SaveFrame->getSaveTypeChinese(SAVE_IMAGE_NOT);
        case 1:
            return SaveFrame->getSaveTypeChinese(SAVE_IMAGE_BMP);
        case 2:
            return SaveFrame->getSaveTypeChinese(SAVE_IMAGE_JPEG);
        case 3:
            return SaveFrame->getSaveTypeChinese(SAVE_IMAGE_PNG);
        default:
            break;
    }
    return "";
}


void GetImgAppAnalysisFrame::run()
{
    int ret = 0;
    uint32_t recvSize = 0;
    uint8_t *imgData = nullptr;
    QImage showImg;
    uint8_t headH = 0;
    uint8_t headL = 0;
    RECV_IMG_INFO_S imgInfo;
    
    while(1)
    {
        analysisDataMutex.lock();
        if(analysisState == ANALYSIS_THREAD_STATE_STOP)
        {
            analysisDataMutex.unlock();
            break;
        }
        analysisDataWaitCondition.wait(&analysisDataMutex);
        switch(analysisState)
        {
            case ANALYSIS_THREAD_STATE_GET_HANDLE:
            {
                if(ringBuffer->getDataSize() < sizeof(RECV_IMG_INFO_S))
                    goto RECV_IMG_THREAD_CONTINUE;
                ringBuffer->read(&headH, 1);
                if (headH != GET_IMG_CMD_HEAD_H)
                    goto RECV_IMG_THREAD_CONTINUE;
                ringBuffer->read(&headL, 1);
                if (headL != GET_IMG_CMD_HEAD_L)
                    goto RECV_IMG_THREAD_CONTINUE;
                
                ringBuffer->read((uint8_t*)&imgInfo + 2, sizeof(RECV_IMG_INFO_S) - sizeof(uint8_t) * 2);
                imgInfo.headH = headH;
                imgInfo.headL = headL;
                analysisState = ANALYSIS_THREAD_STATE_GET_DATA;
            }break;
            case ANALYSIS_THREAD_STATE_GET_DATA:
            {
                if (ringBuffer->getDataSize() < imgInfo.imgSize)
                    goto RECV_IMG_THREAD_CONTINUE;
                imgData = new uint8_t[imgInfo.imgSize];
                recvSize = ringBuffer->read(imgData, imgInfo.imgSize);
                if (recvSize == imgInfo.imgSize)
                {
                    ret = imgDecoder.yuv420decode(imgData, imgInfo.width, imgInfo.height, showImg, getAngle());
                    if (ret >= 0)
                        emit showImgSignal(imgInfo.imgSrcType, showImg);
                    if (getSaveFlag() == SYSTEM_STATUS_OPEN && SaveFrame != nullptr)
                        SaveFrame->addFrameToSave(imgInfo, imgData, showImg, getSaveEncoderType());
                }
                BSD_LOG(LOG_INFO, QString("recv image: %1 , size: %2\n").arg((imgInfo.imgSrcType == IMG_SRC_IR) ? "IR" : "RGB").arg(imgInfo.imgSize));
                delete[] imgData;
                imgData = nullptr;
                analysisState = ANALYSIS_THREAD_STATE_GET_HANDLE;
            }break;
        }
RECV_IMG_THREAD_CONTINUE:
        analysisDataMutex.unlock();
        continue;
    }
}
