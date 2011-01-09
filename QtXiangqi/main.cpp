#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PlayXiangqi");
    QCoreApplication::setOrganizationDomain("playxiangqi.com");
    QCoreApplication::setApplicationName("QtXiangqi");

    MainWindow w;
    w.show();
    return app.exec();
}
