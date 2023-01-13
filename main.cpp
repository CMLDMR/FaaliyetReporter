#include "mainwindow.h"
#include <mongocxx/instance.hpp>



#include <QApplication>

int main(int argc, char *argv[])
{
    mongocxx::instance ins{};

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
