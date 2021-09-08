#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    using namespace selfSpace;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
