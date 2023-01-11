#ifndef BIRIMLISTFORM_H
#define BIRIMLISTFORM_H

#include <QWidget>

#include "personelmanager.h"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextFragment>
//#include <QTest>
#include <QImage>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

#include "faaliyetrapor/faaliyetrapor.h"
#include "meclis/meclismanager.h"
#include "tcmanager.h"

#include "KDReports-qt6/KDReports.h"
#include <KDReports-qt6/KDReportsFrame.h>


#include "configuration.h"
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>



#define log qDebug() << __LINE__ << __FUNCTION__





namespace Ui {
class BirimListForm;
}

class BirimListModel;

class BirimListForm : public QWidget
{
    Q_OBJECT

public:
    explicit BirimListForm(QWidget *parent = nullptr);
    ~BirimListForm();

    void initWidget( SerikBLDCore::DB* _mDB );

    QStringList birimList() const;

    int selectedYear() const;

private slots:
    void on_pushButton_AsagiTasi_clicked();

    void on_pushButton_clicked();

    void on_pushButton_YukariTasi_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_PrintWidget_clicked();

    void on_pushButton_SavePDF_clicked();

    QString toUpperCase( const std::string &word );



    void on_pushButton_SaveList_clicked();

    void on_pushButton_LoadList_clicked();

private:
    Ui::BirimListForm *ui;

    BirimListModel* mModel;

    SerikBLDCore::Faaliyet::Manager* mFaaliyetManager;

    SerikBLDCore::DB* mDB;

    SerikBLDCore::PersonelManager* mPersonelManager;


    KDReports::PreviewWidget *previewWidget;

    KDReports::Report *mReport = nullptr;

    void initHeader();

    void buildReport(SerikBLDCore::Faaliyet::FaaliyetItem *faaliyetItem, int &birimCounter);

    void buildContent(SerikBLDCore::Faaliyet::FaaliyetItem *faaliyetItem);

    void buildMeclisUyeleri();

    void buildLiderler();

    SerikBLDCore::Meclis::UyeManager *mMeclisUyesiManager;
    SerikBLDCore::TCManagerV2 *mTCManager;

    int currentProgressValue;




};


class BirimListModel : public QStandardItemModel, public SerikBLDCore::PersonelManager
{
    Q_OBJECT
public:
    explicit BirimListModel(SerikBLDCore::DB *_mDB);

    void refreshList();

    QStringList getbirimList() const;
    void refreshBirimList();

    QString getPersonelName( const QString &birimName );
    QString getStatuName( const QString &birimName );

    QVector<QString> getBirimListResmi();

private:
    QStringList mBirimList;
};


#endif // BIRIMLISTFORM_H
