#ifndef OTAPACKETVIEW_H
#define OTAPACKETVIEW_H

#include <QWidget>
#include "otapacketleftview.h"
#include "otapacketrightview.h"

namespace Ui {
class OtaPacketView;
}

class OtaPacketView : public QWidget
{
    Q_OBJECT

public:
    explicit OtaPacketView(QWidget *parent = nullptr);
    ~OtaPacketView();

private:
    Ui::OtaPacketView *ui;
};

#endif // OTAPACKETVIEW_H
