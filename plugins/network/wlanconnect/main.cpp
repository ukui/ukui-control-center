#include "wlanconnect.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WlanConnect w;
    w.show();
    return a.exec();
}
