#include "extrainformationdialog.h"
#include "ui_extrainformationdialog.h"

ExtraInformationDialog::ExtraInformationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtraInformationDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Ekstra Bilgiler");
}

ExtraInformationDialog::~ExtraInformationDialog()
{
    delete ui;
}
