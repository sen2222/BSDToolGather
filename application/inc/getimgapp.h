#ifndef GETIMGAPP_H
#define GETIMGAPP_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QByteArray>
#include "ringbuffer.h"
#include "imgdecoder.h"

/***************视频控制命令******************/
#define GET_IMG_CMD_HEAD_H       0xFF        //  头标识0
#define GET_IMG_CMD_HEAD_L       0xAA        //  头标识1
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
    uint8_t     cmd;            // 0x00 关闭 0x01: 开启
}GET_IMG_CMD_S;


#pragma pack(pop)
#ifdef __cplusplus
}
#endif



class GetImgAppSocket : public QObject
{
    Q_OBJECT
public:
    explicit GetImgAppSocket(int id, RingBuffer *ringBuffer = nullptr, QObject *parent = nullptr);
    ~GetImgAppSocket();
    int getConnected();
    int connectToSrever(const QString &ip, const QString &port);
    int disconnectToServer();
    int sendDataToServer(const QByteArray &data);

private:
    bool isIpValid(const QString &ip);
    int convertPortToInt(const QString &port);
    
signals:
    /** 内部信号 */
    void newTcpSocketSignal();
    void deleteTcpSocketSignal();
    void connectToServerSignal(QString ip, int port);
    void disconnectToServerSignal();
    void sendDataToServerSignal(QByteArray data);

    /** 外部信号 */
    void connectSuccessSignal(int id);
    void socketErrorSignal(int id, QAbstractSocket::SocketError error);
    void disconnectSuccessSignal(int id);
    void sendDataSuccessSignal(int id);

private slots:
    /** 内部槽函数 */
    void newTcpSocketSlot();
    void deleteTcpSocketSlot();
    void connecteToServerSlot(QString ip, int port);
    void disconnectToServerSlot();
    void sendDataToServerSlot(QByteArray data);
    void recvDataFromServerSlot();


private:
    QMutex socketMutex;
    QTcpSocket *tcpSocket = nullptr;
    QThread *workThread = nullptr;
    int socketId;
    int connectStatus;              // 0：未连接，1：连接中，2：已连接
    RingBuffer *ringBuffer;
};



/*********************** 视频控制命令类 ************************/
class GetImgAppCmd
{
public:
    explicit GetImgAppCmd();
    ~GetImgAppCmd();
    void setCmd(uint8_t cmd);
    void clearAll();
    QByteArray toByteArray();
private:
    GET_IMG_CMD_S cmdInfo;
};


/*********************** 接收帧数据类 ************************/

typedef enum {
    State_Stop = 0,         // 退出/未运行
    State_Running = 1,      // 正常运行
    State_Pause = 2         // 暂停
}ANALYSIS_THREAD_STATE_E;

class GetImgAppAnalysisFrame : public QThread
{
    Q_OBJECT
public:
    explicit GetImgAppAnalysisFrame(RingBuffer *ringBuffer, QObject *parent = nullptr);
    ~GetImgAppAnalysisFrame() override;
    void stopAnalysis();
    void pauseAnalysis();
    void startAnalysis();

signals:
    void showImgSignal(IMG_SRC_TYPE_E imgSrcType, QImage img);

protected:
    void run() override;

private:
    ImgDecoder imgDecoder;
    RingBuffer *ringBuffer;
    QMutex analysisStateMutex;
    QWaitCondition analysisStateWaitCondition;
    volatile ANALYSIS_THREAD_STATE_E analysisState;

};


#endif
