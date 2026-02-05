#ifndef GETIMGVIEWRIGHT_H
#define GETIMGVIEWRIGHT_H

#include <QWidget>

namespace Ui {
class GetImgViewRight;
}

class GetImgViewRight : public QWidget
{
    Q_OBJECT

public:
    explicit GetImgViewRight(QWidget *parent = nullptr);
    ~GetImgViewRight();

private:
    Ui::GetImgViewRight *ui;
};

#endif // GETIMGVIEWRIGHT_H
