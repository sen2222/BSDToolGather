#ifndef GETIMGVIEWLEFT_H
#define GETIMGVIEWLEFT_H

#include <QWidget>
#include "getimgapp.h"
#include "publictype.h"

#define CONFIG_SECTION_GET_IMG_VIEW_LEFT        "GetImgViewLeft"
#define CONFIG_KEY_DEFAULT_IP1                  "DefaultIP1"
#define CONFIG_KEY_DEFAULT_PORT1                "DefaultPort1"
#define CONFIG_KEY_DEFAULT_IP2                  "DefaultIP2"
#define CONFIG_KEY_DEFAULT_PORT2                "DefaultPort2"




#define IP1_RING_BUFFER_SIZE    (1024 * 1024)
#define IP2_RING_BUFFER_SIZE    (1024 * 1024)



namespace Ui {
class GetImgViewLeft;
}

class GetImgViewLeft : public QWidget
{
    Q_OBJECT

public:
    explicit GetImgViewLeft(QWidget *parent = nullptr);
    ~GetImgViewLeft() override;
    void socketImgInit(void);
    void socketImgDeinit(void);
    void getImgStartSave(GetImgAppSaveFrame *SaveFrame1, GetImgAppSaveFrame *SaveFrame2);
    void getImgStopSave(int id);
    void getImgSetShowAngle(int angle);
    void getImgSetSaveType(int index);
    void getImgSetLedStatus(int id, SYSTEM_STATUS_E status);
    void getImgSetIrcutStatus(int id, SYSTEM_STATUS_E isOn);

private:
    void videoStatusUpdate(int id);

signals:
    void socketUpdateConnectStatus(int id, SOCKET_CONNECT_STATUS_E status);
    void socketUpdateLedStatus(int id, SYSTEM_STATUS_E status);
    void socketUpdateIrcutStatus(int id, SYSTEM_STATUS_E status);

private slots:
    void connectSuccessSlot(int id);
    void disconnectSuccessSlot(int id);
    void socketErrorSlot(int id, QAbstractSocket::SocketError socketError);
    void sendDataSuccessSlot(int id, GetImgAppCmd cmd);

    void showImgSlot(IMG_SRC_TYPE_E imgSrcType, QImage img);

    void on_btnConnect1_clicked();
    void on_btnConnect2_clicked();
    void on_btnOpen1_clicked();
    void on_btnOpen2_clicked();

private:
    Ui::GetImgViewLeft *ui;

    GetImgAppSocket *getImgAppSocket1;
    GetImgAppSocket *getImgAppSocket2;
    GetImgAppAnalysisFrame *getImgAppAnalysisFrame1;
    GetImgAppAnalysisFrame *getImgAppAnalysisFrame2;
    RingBuffer *ip1RingBuffer;
    RingBuffer *ip2RingBuffer;

    SYSTEM_STATUS_E videoStatus1;           
    SYSTEM_STATUS_E videoStatus2;           
};

#endif // GETIMGVIEWLEFT_H
