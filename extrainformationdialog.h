#ifndef EXTRAINFORMATIONDIALOG_H
#define EXTRAINFORMATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ExtraInformationDialog;
}

class ExtraInformationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtraInformationDialog(QWidget *parent = nullptr);
    ~ExtraInformationDialog();

private:
    Ui::ExtraInformationDialog *ui;
};

#endif // EXTRAINFORMATIONDIALOG_H
