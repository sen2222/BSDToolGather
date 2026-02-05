#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("BSDToolGather");
    w.setWindowIcon(QIcon(":/res/test.ico"));
    w.show();
    return a.exec();
}
