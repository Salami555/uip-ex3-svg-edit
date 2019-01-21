#include "mainwindow.h"

#include <QApplication>


int main(int argc, char * argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName("HPI UIP");
    QCoreApplication::setApplicationName("SVG-Edit by MS");
    application.setStyle("windows");

    MainWindow view;
    view.show();

    return application.exec();
}
