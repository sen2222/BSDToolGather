#ifndef GETIMGAPP_H
#define GETIMGAPP_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QByteArray>
#include <QQueue>
#include <QPair>
#include "ringbuffer.h"
#include "imgdecoder.h"
#include "publictype.h"

/***************视频控制命令******************/
#define GET_IMG_CMD_HEAD_H          0xFF        //  头标识0
#define GET_IMG_CMD_HEAD_L          0xAA        //  头标识1
#define GET_IMG_CMD_VIDEO           0xEC        //  视频开关命令
#define GET_IMG_CMD_LED             0xED        //  led开关命令
#define GET_IMG_CMD_IRCUT           0xEB        //  IRUT开关命令

#define GET_IMG_CMD_OPEN            0x01        //  开启视频
#define GET_IMG_CMD_CLOSE           0x00        //  关闭视频





#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#pragma pack(push, 1)
typedef enum
{
    IMG_SRC_IR = 0,
    IMG_SRC_RGB,
    IMG_SRC_BUTT,
}IMG_SRC_TYPE_E;

typedef enum
{
    IMG_ALGO_REG = 0,
    IMG_ALGO_RECOG,
    IMG_ALGO_BUTT,
}IMG_ALGO_TYPE_E;

typedef enum
{
    IMG_TYPE_YUV420 = 0,
    IMG_TYPE_YUV422,
    IMG_TYPE_MJPEG_420,
    IMG_TYPE_MJPEG_422, 
    IMG_TYPE_H265_P,
    IMG_TYPE_H265_I,
    IMG_TYPE_BUTT,
}IMG_TYPE_E;

typedef struct
{
    uint8_t             headH;       // 0xFF
    uint8_t             headL;       // 0xAA
    uint32_t            width;
    uint32_t            height;
    uint32_t            imgSize;
    IMG_SRC_TYPE_E      imgSrcType;
    IMG_TYPE_E          imgType;
    IMG_ALGO_TYPE_E     imgAlgoType;
    uint8_t             crc;
}RECV_IMG_INFO_S;

typedef struct 
{
    uint8_t     headH;          // 0xFF
    uint8_t     headL;          // 0xAA
    uint8_t     cmd;            //  0xEC 视频开关   0xED led开关  0xEB IRCUT开关
    uint8_t     flag;           
}GET_IMG_CMD_S;


#pragma pack(pop)
#ifdef __cplusplus
}
#endif

/*********************** 视频控制命令类 ************************/
class GetImgAppCmd
{
public:
    explicit GetImgAppCmd();
    ~GetImgAppCmd();
    void setCmd(uint8_t cmd);
    void setFlag(uint8_t flag);
    uint8_t getCmd();
    uint8_t getFlag();
    void clearAll();
    QByteArray toByteArray();
private:
    GET_IMG_CMD_S cmdInfo;
};

/*********************** socket类 ************************/
typedef enum
{
    SOCKET_DISCONNECT = 0,         // 断开连接
    SOCKET_CONNECTED,              // 连接中
    SOCKET_CONNECT,                 // 已连接
}SOCKET_CONNECT_STATUS_E;

class GetImgAppSocket : public QObject
{
    Q_OBJECT
public:
    explicit GetImgAppSocket(int id, RingBuffer *ringBuffer = nullptr, QObject *parent = nullptr);
    ~GetImgAppSocket();
    SOCKET_CONNECT_STATUS_E getConnected();
    int connectToSrever(const QString &ip, const QString &port);
    int disconnectToServer();
    int sendDataToServer(GetImgAppCmd &cmd);

private:
    bool isIpValid(const QString &ip);
    int convertPortToInt(const QString &port);
    
signals:
    /** 内部信号 */
    void newTcpSocketSignal();
    void deleteTcpSocketSignal();
    void connectToServerSignal(QString ip, int port);
    void disconnectToServerSignal();
    void sendDataToServerSignal(GetImgAppCmd cmd);

    /** 外部信号 */
    void connectSuccessSignal(int id);
    void socketErrorSignal(int id, QAbstractSocket::SocketError error);
    void disconnectSuccessSignal(int id);
    void sendDataSuccessSignal(int id, GetImgAppCmd cmd);

    void analysisDataUpdateSignal();

private slots:
    /** 内部槽函数 */
    void newTcpSocketSlot();
    void deleteTcpSocketSlot();
    void connecteToServerSlot(QString ip, int port);
    void disconnectToServerSlot();
    void sendDataToServerSlot(GetImgAppCmd cmd);
    void recvDataFromServerSlot();


private:
    QMutex socketMutex;
    QTcpSocket *tcpSocket = nullptr;
    QThread *workThread = nullptr;
    int socketId;
    SOCKET_CONNECT_STATUS_E connectStatus;              // 0：未连接，1：连接中，2：已连接
    RingBuffer *ringBuffer;

    QQueue<GetImgAppCmd> cmdQueue;
    QMutex cmdMutex;
};





/**************************保存帧数据类****************************/
class GetImgViewRight;

typedef enum {
    SAVE_THREAD_STATE_STOP = 0,
    SAVE_THREAD_STATE_GET_INFO = 1,
} SAVE_THREAD_STATE_E;

typedef enum {
    SAVE_IMAGE_NOT = 0,
    SAVE_IMAGE_BMP = 1,
    SAVE_IMAGE_JPEG = 2,
    SAVE_IMAGE_PNG = 3,
} SAVE_THREAD_TYPE_E;

typedef struct
{
    RECV_IMG_INFO_S     imgInfo;
    QByteArray          yuvData;
    QImage              imgData;
    SAVE_THREAD_TYPE_E  saveType;
}SAVE_THREAD_DATA_S;

typedef struct 
{
    QString IRSavePath;
    QString RGBSavePath;
    QString BMPSavePath;
    QString JPEGSavePath;
    QString PNGSavePath;
    GetImgViewRight *rightView;
}SAVE_THREAD_SAVE_PATH_S;

class GetImgAppSaveFrame : public QThread
{
    Q_OBJECT
public:
    explicit GetImgAppSaveFrame(SAVE_THREAD_SAVE_PATH_S &savePath, QObject *parent = nullptr);
    ~GetImgAppSaveFrame() override;

    void stopSave();

    void addFrameToSave(RECV_IMG_INFO_S &imgInfo, uint8_t *data, QImage &image, SAVE_THREAD_TYPE_E saveType);
    void packCurrentSaveDir();
    QString getSaveTypeChinese(SAVE_THREAD_TYPE_E saveType);
private:
    void clearSaveQueue();
    int saveFrameToFile(SAVE_THREAD_DATA_S *frame);
    int WriteBmpFromARGB(const QString &path, const QImage &img);
    SAVE_THREAD_STATE_E getSaveState();
    void run() override;

    QQueue<SAVE_THREAD_DATA_S *> saveQueue;
    QMutex saveMutex;
    QWaitCondition saveWaitCondition;
    SAVE_THREAD_STATE_E saveState;
    SAVE_THREAD_SAVE_PATH_S savePath;
};


/*********************** 接收帧数据类 ************************/

typedef enum {
    ANALYSIS_THREAD_STATE_STOP = 0,             // 退出/未运行
    ANALYSIS_THREAD_STATE_GET_HANDLE = 1,       // 获取句柄
    ANALYSIS_THREAD_STATE_GET_DATA = 2,         // 获取数据帧
}ANALYSIS_THREAD_STATE_E;

class GetImgAppAnalysisFrame : public QThread
{
    Q_OBJECT
public:
    explicit GetImgAppAnalysisFrame(RingBuffer *ringBuffer, QObject *parent = nullptr);
    ~GetImgAppAnalysisFrame() override;
    void stopAnalysis();
    void startSaveFrame(GetImgAppSaveFrame *SaveFrame);
    void stopSaveFrame();
    void setShowAngle(int angle);
    void setSaveType(int index);
    QString getSaveEncoderChinese(int index);

signals:
    void showImgSignal(IMG_SRC_TYPE_E imgSrcType, QImage img);
public slots:
    void analysisDataUpdateSlot();
    
protected:
    void run() override;

private:
    ImgDecoder imgDecoder;
    RingBuffer *ringBuffer;
    GetImgAppSaveFrame *SaveFrame = nullptr;
    QMutex saveFlagMutex;
    SYSTEM_STATUS_E saveFlag;
    
    ANALYSIS_THREAD_STATE_E analysisState;
    QMutex analysisDataMutex;
    QWaitCondition analysisDataWaitCondition;

    I2D_ANGLE_E angle;
    QMutex angleMutex;

    SAVE_THREAD_TYPE_E saveEncoderType;
    QMutex saveEncoderTypeMutex;

    void setSaveFlag(SYSTEM_STATUS_E flag);
    SYSTEM_STATUS_E getSaveFlag();
    I2D_ANGLE_E getAngle();
    void setAngle(I2D_ANGLE_E angle);
    SAVE_THREAD_TYPE_E getSaveEncoderType();
    void setSaveEncoderType(SAVE_THREAD_TYPE_E saveEncoderType);
};

#endif
