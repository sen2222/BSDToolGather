#ifndef GETIMGVIEWLEFT_H
#define GETIMGVIEWLEFT_H

#include <QWidget>
#include "getimgapp.h"

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

private:

private slots:
    void connectSuccessSlot(int id);
    void disconnectSuccessSlot(int id);
    void socketErrorSlot(int id, QAbstractSocket::SocketError socketError);
    void sendDataSuccessSlot(int id);

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

    int videoStatus1;
    int videoStatus2;
};

#endif // GETIMGVIEWLEFT_H
