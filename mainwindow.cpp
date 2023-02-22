#include "mainwindow.h"
#include "ui_mainwindow.h"

//static const char* _url = "mongodb://<>SeriKbeltR<>:><><S_EKkrikneltR<>TR>1926><@192.168.0.11:27018/?authSource=SERIKBELTR";

namespace SBLDKeys {

    static const std::string DB{"SERIKBELTR"};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    this->showMaximized();

    try {
        mClient = new mongocxx::client(mongocxx::uri(_url));
    } catch (mongocxx::exception& e) {
        log << "MongoDB Connection Error: " << e.what();
        return;
    }

    db = mClient->database(SBLDKeys::DB);

    mDB = new SerikBLDCore::DB(&db);

    ui->birimView->initWidget (mDB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

