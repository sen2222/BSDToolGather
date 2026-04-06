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
    void leftWidgetInit(void);
    void leftWidgetDeinit(void);
    void rightWidgetInit(void);
    void rightWidgetDeinit(void);

private:
    Ui::GetImgView *ui;
};

#endif // GETIMGVIEW_H
