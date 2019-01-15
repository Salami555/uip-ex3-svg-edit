#include "mainwindow.h"

#include <QApplication>

#include "controller.h"
#include "model.h"


int main(int argc, char * argv[])
{
    QApplication application(argc, argv);

    Model model;

    Controller controller;
    controller.setModel(&model);

    MainWindow view;
    view.setModel(&model);
    view.setController(&controller);
    view.show();

    return application.exec();
}
