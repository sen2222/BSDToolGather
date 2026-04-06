#ifndef GETIMGVIEWRIGHT_H
#define GETIMGVIEWRIGHT_H

#include <QWidget>
#include <QDir>
#include <QString>

#include "getimgviewleft.h"
#include "getimgapp.h"
#include "publictype.h"

#define CONFIG_SECTION_GET_IMG_VIEW_RIGHT       "GetImgViewRight"
#define CONFIG_KEY_DEFAULT_IR_SAVE_PATH         "IRSavePath"
#define CONFIG_KEY_DEFAULT_RGB_SAVE_PATH        "RGBSavePath"
#define CONFIG_KEY_DEFAULT_BMP_SAVE_PATH        "BMPSavePath"
#define CONFIG_KEY_DEFAULT_JPEG_SAVE_PATH       "JPEGSavePath"
#define CONFIG_KEY_DEFAULT_PNG_SAVE_PATH        "PNGSavePath"

namespace Ui {
class GetImgViewRight;
}


class GetImgViewRight : public QWidget
{
    Q_OBJECT

public:
    explicit GetImgViewRight(QWidget *parent = nullptr);
    ~GetImgViewRight();
    void setLeftView(GetImgViewLeft *getImgViewLeft);
    void SaveThreadInit(void);
    void SaveThreadDeinit(void);
    uint32_t getImgSaveNum(IMG_SRC_TYPE_E imgSrcType);
    void addImgSaveNum(IMG_SRC_TYPE_E imgSrcType);

private:
    void clearImgSaveNum(void);
    void setSaveStatus(int ipId, SYSTEM_STATUS_E saveStatus);
    SYSTEM_STATUS_E getSaveStatus(int ipId);
    void setLedStatus(int ipId, SYSTEM_STATUS_E ledStatus);
    SYSTEM_STATUS_E getLedStatus(int ipId);
    void setIrcutStatus(int ipId, SYSTEM_STATUS_E ircutStatus);
    SYSTEM_STATUS_E getIrcutStatus(int ipId);


    void setSaveBtnStatus(int ipId, SYSTEM_STATUS_E saveStatus);


private slots:
    void on_btnIP1Save_clicked();
    void on_btnIP2Save_clicked();
    void on_btnClearNum_clicked();
    void on_btnPacket_clicked();
    void on_btnLedSwitch1_clicked();
    void on_btnLedSwitch2_clicked();
    void on_btnIrcutSwitch1_clicked();
    void on_btnIrcutSwitch2_clicked();

    void on_boxI2d_currentIndexChanged(int index);
    void on_boxSaveDedoder_currentIndexChanged(int index);

    void socketUpdateConnectStatusSlot(int id, SOCKET_CONNECT_STATUS_E status);
    void socketUpdateLedStatusSlot(int id, SYSTEM_STATUS_E status);
    void socketUpdateIrcutStatusSlot(int id, SYSTEM_STATUS_E status);

private:
    Ui::GetImgViewRight *ui;
    GetImgViewLeft *getImgViewLeft;

    SAVE_THREAD_SAVE_PATH_S savePath;
    GetImgAppSaveFrame *getImgAppSaveFrame1;
    GetImgAppSaveFrame *getImgAppSaveFrame2;

    uint32_t saveIRNum;
    uint32_t saveRGBNum;
    QMutex saveNumMutex;
    QMutex saveStateMutex;
    QMutex ledMutex;
    QMutex ircutMutex;
    SYSTEM_STATUS_E ip1SaveStatus;
    SYSTEM_STATUS_E ip2SaveStatus;
    SYSTEM_STATUS_E ip1LedStatus;
    SYSTEM_STATUS_E ip2LedStatus;
    SYSTEM_STATUS_E ip1IrcutStatus;
    SYSTEM_STATUS_E ip2IrcutStatus;
};

#endif // GETIMGVIEWRIGHT_H
