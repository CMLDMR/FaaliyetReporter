#include "mainwindow.h"
#include <mongocxx/instance.hpp>



#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    mongocxx::instance ins{};

    MainWindow w;
    w.show();
    return a.exec();
}
