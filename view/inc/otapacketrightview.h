#ifndef OTAPACKETRIGHTVIEW_H
#define OTAPACKETRIGHTVIEW_H

#include <QWidget>
#include "otapacketleftview.h"
#include "otapacketapp.h"



namespace Ui {
class OtaPacketRightView;
}

class OtaPacketRightView : public QWidget
{
    Q_OBJECT

public:
    explicit OtaPacketRightView(QWidget *parent = nullptr);
    ~OtaPacketRightView();
    void setLeftView(OtaPacketLeftView* leftView);
    void clearAllUserParam();

private slots:
    void on_btnPacket_clicked();

private:
    Ui::OtaPacketRightView *ui;
    OtaPacketLeftView*  m_leftView;
    OtaPacketApp*       m_packetApp;
};

#endif // OTAPACKETRIGHTVIEW_H
