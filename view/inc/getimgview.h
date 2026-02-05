#ifndef GETIMGVIEW_H
#define GETIMGVIEW_H

#include <QWidget>

namespace Ui {
class GetImgView;
}

class GetImgView : public QWidget
{
    Q_OBJECT

public:
    explicit GetImgView(QWidget *parent = nullptr);
    ~GetImgView();

private:
    Ui::GetImgView *ui;
};

#endif // GETIMGVIEW_H
