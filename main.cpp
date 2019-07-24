
#include "VLCFramePaint.h"
#include <QtWidgets/QApplication>
#include "widget.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    //VLCFramePaint w;
    w.show();
    return a.exec();
}