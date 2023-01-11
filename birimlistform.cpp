#include "birimlistform.h"
#include "ui_birimlistform.h"


#include <QDebug>


BirimListForm::BirimListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BirimListForm)
{
    ui->setupUi(this);

    previewWidget = new KDReports::PreviewWidget(this);
    ui->verticalLayout_PReview->addWidget (previewWidget);

}

BirimListForm::~BirimListForm()
{
    delete ui;
}

void BirimListForm::initWidget(SerikBLDCore::DB *_mDB)
{

    mDB = _mDB;

    mFaaliyetManager = new SerikBLDCore::Faaliyet::Manager(_mDB);

    mPersonelManager = new SerikBLDCore::PersonelManager(_mDB);

    mMeclisUyesiManager = new SerikBLDCore::Meclis::UyeManager(_mDB);

    mTCManager = new SerikBLDCore::TCManagerV2(_mDB);

    mModel = new BirimListModel(_mDB);
    ui->tableView->setModel (mModel);

}

BirimListModel::BirimListModel(SerikBLDCore::DB *_mDB)
    :    SerikBLDCore::PersonelManager(_mDB)
{
    this->refreshList ();
}

void BirimListModel::refreshList()
{
    this->removeRows (0,this->rowCount ());

    auto birimList = this->getBirimListResmi();

    this->setHorizontalHeaderLabels (QStringList()<< "Birimler"<<"Personel"<<"Müdür/Vekil");

    SerikBLDCore::IK::Personel personelFilter;
    personelFilter.setStatu (SerikBLDCore::IK::Statu::Mudur.c_str ());

    auto mudurList = this->List(personelFilter,SerikBLDCore::FindOptions().setLimit (100));

    for( const auto &item : birimList ){
        auto standartItem = new QStandardItem();
        standartItem->setText (item);
        this->insertRow (0,standartItem);

        for( const auto &mudur : mudurList ){
            if( mudur.Birim () == item ){
                this->setItem (0,1,new QStandardItem(mudur.AdSoyad ()));
                break;
            }
        }
        this->setItem (0,2,new QStandardItem("Müdür/Müdür Vekili"));
    }
    this->refreshBirimList ();
}

QStringList BirimListModel::getbirimList() const
{
    return this->mBirimList;
}


void BirimListModel::refreshBirimList()
{
    mBirimList.clear ();

    for( auto i = 0 ; i < this->rowCount () ; i++ ){
        mBirimList.append (this->item (i,0)->text ());
    }
}

QString BirimListModel::getPersonelName(const QString &birimName)
{
    QString str;
    for( int i = 0 ; i < this->rowCount () ; i++ ){
        if( birimName == this->item (i,0)->text () ){
            if( this->item (i,1) ){
                str = this->item (i,1)->text ();
                break;
            }
        }
    }
    return str;
}

QString BirimListModel::getStatuName(const QString &birimName)
{
    QString str;
    for( int i = 0 ; i < this->rowCount () ; i++ ){
        if( birimName == this->item (i,0)->text () ){
            if( this->item (i,2) ){
                str = this->item (i,2)->text ();
                break;
            }
        }
    }
    return str;
}

void BirimListForm::on_pushButton_AsagiTasi_clicked()
{
    if( ui->tableView->currentIndex ().row () >= mModel->rowCount () ){
        return;
    }
    auto list = mModel->takeRow (ui->tableView->currentIndex ().row ());
    mModel->insertRow (ui->tableView->currentIndex ().row ()+1,list);
    mModel->refreshBirimList ();
}

QStringList BirimListForm::birimList() const
{
    return this->mModel->getbirimList ();
}

int BirimListForm::selectedYear() const
{
    return ui->spinBox_yil->value ();
}

void BirimListForm::on_pushButton_clicked()
{
    this->mModel->refreshList ();
}

void BirimListForm::on_pushButton_YukariTasi_clicked()
{
    if( ui->tableView->currentIndex ().row () <= 0 ){
        return;
    }
    auto list = mModel->takeRow (ui->tableView->currentIndex ().row ());
    mModel->insertRow (ui->tableView->currentIndex ().row ()-1,list);
    mModel->refreshBirimList ();
}




void BirimListForm::on_pushButton_2_clicked()
{

    auto _list = mFaaliyetManager->ListFaaliyetItem (this->selectedYear ());

    QVector<SerikBLDCore::Faaliyet::FaaliyetItem> list;

    for( const auto &item : _list ){
        list.append (item);
    }

    QVector<SerikBLDCore::Faaliyet::FaaliyetItem> newlist;

    for( int i = 0 ; i < mModel->rowCount () ; i++ ){
        for( const auto &item : list ){
            if( mModel->item (i,0)->text ().toStdString () == item.getBirim () ){
                newlist.push_back (item);
                break;
            }
        }
    }

    if( mReport ){
        delete mReport;
        mReport = new KDReports::Report(this);

    }else{
        mReport = new KDReports::Report(this);
    }

    mReport->setWatermarkImage (QImage("logo.png"));

    this->initHeader ();
    this->buildMeclisUyeleri ();


    QMap<QString,QString> content;
    QStringList contentList;


    int birimCounter = 1;
    int itemCounterForProgressBar = 0;
    for( const auto &faaliyetItem : list ){
        auto arr = faaliyetItem.getFaaliyetList ();

        auto countElement = std::distance(arr.begin (),arr.end ());
        itemCounterForProgressBar += countElement;

    }
    currentProgressValue = 0;

    ui->progressBar->setMaximum (itemCounterForProgressBar);


    KDReports::TextElement icindekiler1;
    icindekiler1.setText ("İçindekiler");
    icindekiler1.setFontFamily ("Cambria");
    icindekiler1.setPointSize (13);
    mReport->addElement (icindekiler1);
    mReport->addVerticalSpacing (0.05);

    for( const auto &faaliyetItem : newlist ){
        this->buildContent (new SerikBLDCore::Faaliyet::FaaliyetItem(faaliyetItem));
    }
    mReport->addPageBreak ();

    for( const auto &faaliyetItem : newlist ){
        buildReport (new SerikBLDCore::Faaliyet::FaaliyetItem(faaliyetItem),birimCounter);
    }

    ui->progressBar->setValue (ui->progressBar->maximum ());

    previewWidget->setReport (mReport);

}


// Sadece Bir Tane Birimin Dökümanı
void BirimListForm::on_pushButton_PrintWidget_clicked()
{

    auto currentRow = ui->tableView->currentIndex ().row ();
    if( currentRow < 0 ){
        log << "No Selected Element";
        return;
    }

    auto faaliyetItem = mFaaliyetManager->getFaaliyetItem (mModel->item (currentRow,0)->text ().toStdString (),this->selectedYear ());

    if( !faaliyetItem ){
        return;
    }

    if( mReport ){
        delete mReport;
        mReport = new KDReports::Report(this);

    }else{
        mReport = new KDReports::Report(this);
    }

    mReport->setWatermarkImage (QImage("logo.png"));

    this->initHeader ();

    auto arr = faaliyetItem->getFaaliyetList ();

    auto countElement = std::distance(arr.begin (),arr.end ());
    ui->progressBar->setMaximum (countElement);
    currentProgressValue = 0;

    int birimCounter = 1;

    //    this->buildLiderler();

    this->buildMeclisUyeleri ();

    //    KDReports::TextElement icindekiler1;
    //    icindekiler1.setText ("İçindekiler");
    //    icindekiler1.setFontFamily ("Cambria");
    //    icindekiler1.setPointSize (13);
    //    mReport->addElement (icindekiler1);
    //    mReport->addVerticalSpacing (0.05);

    //    this->buildContent (faaliyetItem.get ());

    //    mReport->addPageBreak ();

    buildReport (faaliyetItem.get (),birimCounter);

    ui->progressBar->setValue (ui->progressBar->maximum ());

    previewWidget->setReport (mReport);

}

void BirimListForm::on_pushButton_SavePDF_clicked()
{

    if( !mReport ){
        return;
    }

    auto saveFileName = QFileDialog::getSaveFileName (this,"Konum Seç","","*.pdf");

    QFileInfo infoFileName;
    infoFileName.setFile (saveFileName);

    if( QFileInfo(saveFileName).suffix () != "pdf" ){
        saveFileName.append (".pdf");
    }

    mReport->exportToFile (saveFileName);

}

QString BirimListForm::toUpperCase(const std::string &word)
{

    QString mWord = QString::fromStdString (word);

    QString _mWord;
    for( auto _char : mWord ){

        bool proccesed = false;
        if( _char == 'i' ){
            _mWord.append ("İ");
            proccesed = true;
        }

        if( !proccesed ){
            _mWord.append (_char.toUpper ());
        }


    }

    return _mWord;

}

void BirimListForm::buildReport(SerikBLDCore::Faaliyet::FaaliyetItem *faaliyetItem, int &birimCounter)
{

    {

        KDReports::TextElement title1(QString("İÇ KONTROL GÜVENCE BEYANI"));
        title1.setFontFamily ("Cambria");
        title1.setPointSize (15);
        mReport->addElement (title1,Qt::AlignmentFlag::AlignLeft);
        mReport->addVerticalSpacing (0.1);

        {
            KDReports::TextElement title(QString("\tÜst yönetici olarak yetkim dahilinde;"));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }

        {
            KDReports::TextElement title(QString("\tBu raporda yer alan bilgilerin güvenilir, tam ve doğru olduğunu beyan ederim. Bu raporda açıklanan faaliyetler için bütçe ile tahsis edilmiş kaynakların, planlanmış amaçlar doğrultusunda ve iyi mali yönetim ilkelerine uygun olarak kullanıldığını ve iç kontrol sisteminin işlemlerin yasallık ve düzenliliğine ilişkin yeterli güvenceyi sağladığını bildiririm."));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        {
            KDReports::TextElement title(QString("\tBu güvence, üst yönetici olarak sahip olduğum bilgi ve değerlendirmeler, iç kontroller, iç denetçi raporları ile Sayıştay raporları gibi bilgim dahilindeki hususlara dayanmaktadır. Burada raporlanmayan, idarenin menfaatlerine zarar veren herhangi bir husus hakkında bilgim olmadığını beyan ederim. 31/Aralık/2021"));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        mReport->addVerticalSpacing (15);

        KDReports::TableElement tableElement;
        tableElement.setHeaderColumnCount (3);
        tableElement.setWidth (100,KDReports::Percent);
        tableElement.setBorder (0);

        KDReports::TextElement mudurAdSoyadText;
        mudurAdSoyadText.setText (this->mModel->getPersonelName (faaliyetItem->getBirim ().c_str ()));
        mudurAdSoyadText.setFontFamily ("Tahoma");
        mudurAdSoyadText.setPointSize (12);

        KDReports::TextElement birimText;
        birimText.setText (this->mModel->getStatuName (faaliyetItem->getBirim ().c_str ()));
        birimText.setFontFamily ("Tahoma");
        birimText.setPointSize (12);

        KDReports::TextElement harcamaText;
        harcamaText.setText ("Harcama Yetkilisi");
        harcamaText.setFontFamily ("Tahoma");
        harcamaText.setPointSize (12);


        tableElement.cell (0,2).addElement (KDReports::TextElement(mudurAdSoyadText),Qt::AlignCenter);
        tableElement.cell (1,2).addElement (KDReports::TextElement(birimText),Qt::AlignCenter);
        tableElement.cell (2,2).addElement (KDReports::TextElement(harcamaText),Qt::AlignCenter);

        mReport->addElement (tableElement,Qt::AlignJustify);

        mReport->addPageBreak ();

    }




    std::vector<SerikBLDCore::Faaliyet::RaporItem> arr;

    try {

        auto listAr = faaliyetItem->view()["faaliyet"].get_array().value;

        for( const auto &item : listAr ){

            SerikBLDCore::Faaliyet::RaporItem _item;
            _item.setDocumentView(item.get_document().view());
            arr.push_back(_item);

        }

    } catch (bsoncxx::exception &e) {
        log << e.what();
        return;
    }


    //    auto arr = faaliyetItem->getFaaliyetList ();

    int idCounter = 0;
    {
        mReport->addVerticalSpacing (30);

        KDReports::TextElement baslik;
        std::string baslikStr = toUpperCase (faaliyetItem->getBirim ()).toStdString ()/*faaliyetItem->getBirim ()*/;
        if( faaliyetItem->getBirim () == "Başkanlık" ){
            baslikStr = "GENEL BİLGİLER";
        }
        baslik.setText (QString("%1. ").arg (birimCounter) + baslikStr.c_str ());
        baslik.setFontFamily ("Cambria");
        baslik.setPointSize (18);
        baslik.setBold (true);
        baslik.setTextColor (QColor(100,100,100));

        mReport->addElement (baslik);


        mReport->addVerticalSpacing (1);
        mReport->associateTextValue (QString("id_%1_%2").arg(faaliyetItem->oid ().value ().to_string ().c_str ()).arg (idCounter),QString::number (mReport->numberOfPages ()));
        idCounter++;
    }



    int baslikCounter = 0;
    int altBaslikCOunter = 0;
    for( const auto &item : arr ){

        ui->progressBar->setValue (currentProgressValue++);


        if( item.isBaslik () ){

            KDReports::Frame frame;

            baslikCounter++;
            KDReports::TextElement baslik;
            baslik.setText (QString("%1.%2.  ").arg (birimCounter).arg (baslikCounter) +item.getText ().c_str ());
            baslik.setFontFamily ("Tahoma");
            baslik.setPointSize (14);
            baslik.setTextColor (QColor(100,100,100));
            frame.addElement (baslik);
            altBaslikCOunter = 0;


            //            frame.setLeftMargin (0);
            frame.setBorder (0);
            frame.setPadding (0);
            //            frame.setTopMargin (0);
            //            frame.setBottomMargin (0);
            frame.setBorder (0);

            mReport->addElement (frame);

            mReport->associateTextValue (QString("id_%1_%2").arg(faaliyetItem->oid ().value ().to_string ().c_str ()).arg (idCounter),QString::number (mReport->numberOfPages ()));
            idCounter++;

        }

        if( item.isPageBreak () ){
            mReport->addPageBreak ();
        }





        if( item.isAltBaslik () ){
            altBaslikCOunter++;

            KDReports::Frame frame;


            KDReports::TextElement altbaslik;
            altbaslik.setText (QString("     %1.%2.%3  ").arg (birimCounter).arg (baslikCounter).arg (altBaslikCOunter) + item.getText ().c_str ());
            altbaslik.setFontFamily ("Tahoma");
            altbaslik.setPointSize (11);
            altbaslik.setBold (true);
            altbaslik.setTextColor (QColor(75,75,75));

            //            frame.addElement (altbaslik,Qt::AlignJustify);

            //            frame.setLeftMargin (0);
            frame.setBorder (0);
            frame.setPadding (0);
            //            frame.setTopMargin (-5);
            //            frame.setBottomMargin (0);
            frame.setBorder (0);

            //            mReport->addElement (frame);
            mReport->addElement (altbaslik,Qt::AlignJustify);


            mReport->associateTextValue (QString("id_%1_%2").arg(faaliyetItem->oid ().value ().to_string ().c_str ()).arg (idCounter),QString::number (mReport->numberOfPages ()));
            idCounter++;

        }






        if( item.isParagraf () ){

            QString htmlStr = item.getText ().c_str ();

            //                KDReports::Frame frame;

            QTextDocument editor;
            editor.setHtml (htmlStr);
            QFont font;
            font.setFamily ("Tahoma");
            font.setPointSize (11);
            editor.setDefaultFont (font);
            editor.setDefaultTextOption (QTextOption(Qt::AlignmentFlag::AlignJustify));
            KDReports::HtmlElement paragraf;
            paragraf.setHtml (editor.toHtml ());
            mReport->addElement (paragraf,Qt::AlignJustify);

        }




        if( item.isImg () ){
            mReport->addVerticalSpacing (0.3);

            SerikBLDCore::Faaliyet::ImgItem imgItem;
            imgItem.setDocumentView (item.view ());

            auto path = mDB->downloadFile (imgItem.getImgOid ().c_str ());

            QImage img;
            if( img.load (path.c_str ())){


                KDReports::ImageElement imgElement(img);
                imgElement.setWidth (100,KDReports::Percent);


                mReport->addElement (imgElement,Qt::AlignCenter);


                KDReports::TextElement imgTitle;
                //  10 Karakter Boşluk
                imgTitle.setText (item.getText ().c_str ());
                imgTitle.setFontFamily ("Tahoma");
                imgTitle.setPointSize (11);
                mReport->addElement (imgTitle,Qt::AlignCenter);

                mReport->addVerticalSpacing (0.3);
            }




        }




        if( item.isTable () ){
            mReport->addVerticalSpacing (1);
            SerikBLDCore::Faaliyet::TableItem tableItem;
            tableItem.setDocumentView (item.view ());

            {
                KDReports::TextElement textElement;
                textElement.setText (tableItem.getText ().c_str ());
                textElement.setFontFamily ("Arial");
                textElement.setPointSize (11);
                textElement.setBold (true);
                mReport->addElement (textElement,Qt::AlignCenter);
            }


            KDReports::TableElement tableElement;
            tableElement.setHeaderColumnCount (tableItem.column ());

            int jj = 0;
            for( const auto &headerText : tableItem.headers () ){

                KDReports::TextElement textElement;
                textElement.setText (headerText.c_str ());
                textElement.setFontFamily ("Arial");
                textElement.setPointSize (10);
                textElement.setBold (true);
                tableElement.cell (0,jj++).addElement (textElement);
            }

            for( int i = 0 ; i < tableItem.row () ; i++ ){
                for( int j = 0 ; j < tableItem.column () ; j++ ){
                    KDReports::TextElement textElement;
                    textElement.setText (tableItem.cell (i,j).c_str ());
                    textElement.setFontFamily ("Arial");
                    textElement.setPointSize (9);
                    tableElement.cell (i+1,j).addElement (textElement);
                    if( i%2 == 0 ){
                        tableElement.cell (i,j).setBackground (QBrush(QColor(225,225,245)));
                    }
                }
            }
            tableElement.setBorder (.25);
            tableElement.setBorderBrush (QBrush(QColor(75,75,75)));
            tableElement.setWidth (100,KDReports::Percent);

            mReport->addElement (tableElement);

            mReport->addVerticalSpacing (5);
        }
    }
    birimCounter++;
    mReport->addPageBreak ();
}

void BirimListForm::buildContent(SerikBLDCore::Faaliyet::FaaliyetItem *faaliyetItem )
{

    KDReports::TableElement contentTable;
    contentTable.setHeaderColumnCount (2);
    contentTable.setBorder (0);

    int i = 0;
    int id = 0 ;

    auto arr = faaliyetItem->getFaaliyetList ();

    {
        std::string baslikStr = toUpperCase (faaliyetItem->getBirim ()).toStdString ()/*faaliyetItem->getBirim ()*/;
        if( faaliyetItem->getBirim () == "Başkanlık" ){
            baslikStr = "GENEL BİLGİLER";
        }

        KDReports::TextElement tocElement(baslikStr.c_str ());
        tocElement.setFontFamily ("Cambria");
        tocElement.setPointSize (10);
        tocElement.setBold (true);

        contentTable.cell (i,0).addElement (tocElement,Qt::AlignLeft);

        KDReports::TextElement indexText(" index ");
        indexText.setId( QString("id_%1_%2").arg (faaliyetItem->oid ().value ().to_string ().c_str ()).arg (id++));
        contentTable.cell (i,1).addElement (indexText,Qt::AlignRight);

    }

    for( const auto &item : arr ){

        if( item.isBaslik () ){
            KDReports::TextElement tocElement(QString("   ") + item.getText ().c_str ());
            tocElement.setFontFamily ("Cambria");
            tocElement.setPointSize (10);

            contentTable.cell (i,0).addElement (tocElement,Qt::AlignLeft);

            KDReports::TextElement indexText(" index ");
            indexText.setId( QString("id_%1_%2").arg (faaliyetItem->oid ().value ().to_string ().c_str ()).arg (id++));
            contentTable.cell (i,1).addElement (indexText,Qt::AlignRight);

        }

        if( item.isAltBaslik () ){
            KDReports::TextElement tocElement(QString("       ") + item.getText ().c_str ());
            tocElement.setFontFamily ("Arial");
            tocElement.setPointSize (9);
            tocElement.setItalic (true);

            contentTable.cell (i,0).addElement (tocElement,Qt::AlignLeft);

            KDReports::TextElement indexText(" index ");
            indexText.setId( QString("id_%1_%2").arg (faaliyetItem->oid ().value ().to_string ().c_str ()).arg (id++));
            contentTable.cell (i,1).addElement (indexText,Qt::AlignRight);

        }



    }

    contentTable.setWidth (100,KDReports::Unit::Percent);
    mReport->addElement (contentTable);



}

void BirimListForm::buildMeclisUyeleri()
{

    this->buildLiderler ();

    SerikBLDCore::FindOptions findOptions;
    findOptions.setLimit (100);

    SerikBLDCore::Item sort("");
    sort.append("index",1);

    findOptions.setSort (sort);

    SerikBLDCore::Meclis::MeclisUyesi uyeFilter;
    uyeFilter.setDonemAdi (ui->comboBox_Donem->currentText ().toStdString ());

    auto uyeList = mMeclisUyesiManager->List (uyeFilter,findOptions);

    QStringList partiList;

    for( const auto &item : uyeList ){
        SerikBLDCore::Meclis::MeclisUyesi meclisUyesi(item);
        partiList.append (item.partiAdi ());
    }

    partiList.removeDuplicates ();

    partiList.clear ();

    partiList.append ("Adalet ve Kalkınma Partisi");
    partiList.append ("Demokratik Sol Parti");
    partiList.append ("Milliyetçi Hareket Partisi");
    partiList.append ("İyi Parti");
    partiList.append ("Cumhuriyet Halk Partisi");

    //    partiList.append ("Bağımsız");

    for( const auto &parti : partiList ){

        if( parti == "Milliyetçi Hareket Partisi" ){
            //                        mReport->addElement (KDReports::HLineElement());
            mReport->addPageBreak ();
        }

        if( parti == "Demokratik Sol Parti" ){
            //            mReport->addPageBreak ();
            mReport->addElement (KDReports::HLineElement());
        }

        if( parti == "İyi Parti" ){
            //            mReport->addPageBreak ();
            //                    mReport->addVerticalSpacing (25);
            mReport->addElement (KDReports::HLineElement());
        }

        if( parti == "Cumhuriyet Halk Partisi" ){
            mReport->addPageBreak ();
            //            mReport->addElement (KDReports::HLineElement());
        }

        //        if( parti == "Bağımsız" ){
        //            mReport->addVerticalSpacing (45);
        //            mReport->addElement (KDReports::HLineElement());
        //        }

        KDReports::TextElement element(parti);

        element.setBold (true);
        element.setFontFamily ("Cambria");
        element.setPointSize (13);

        mReport->addElement (element,Qt::AlignmentFlag::AlignCenter);

        KDReports::TableElement table;

        int i = 0;
        int j = 0;
        for( const auto &item : uyeList ){


            if( item.partiAdi () == parti ){

                auto tcItem = mTCManager->Load_byOID (item.tcOid ().toStdString ());

                if( tcItem ){

                    auto imgPath = mTCManager->downloadFile (tcItem.value ()->FotoOid ());

                    KDReports::Frame frame;

                    QImage img;
                    if( img.load (imgPath.c_str ()) ){
                        img = img.scaledToWidth (100,Qt::SmoothTransformation);
                        frame.addElement (KDReports::ImageElement(img),Qt::AlignCenter);
                    }
                    KDReports::TextElement adSoyadElement(tcItem.value ()->AdSoyad ());
                    adSoyadElement.setBold (true);
                    adSoyadElement.setFontFamily ("Arial");
                    adSoyadElement.setPointSize (9);
                    frame.addElement (adSoyadElement,Qt::AlignCenter);

                    ///Komisyon Üyelikleri
                    //                    for( const auto &komisyon : item.komisyonUyelikleri () ){

                    //                        KDReports::TextElement komisyonText(komisyon);
                    //                        komisyonText.setBold (false);
                    //                        komisyonText.setItalic (true);
                    //                        komisyonText.setFontFamily ("Arial");
                    //                        komisyonText.setPointSize (8);
                    //                        frame.addElement (komisyonText,Qt::AlignCenter);

                    //                    }

                    frame.setBorder (0);
                    //                    frame.setTopMargin (-5);

                    table.cell (i,j).addElement (frame,Qt::AlignCenter);
                }else{
                    table.cell (i,j).addElement (KDReports::TextElement("null_ptr"));
                }


                j++;
                if( j >= 4 ){
                    j = 0;
                    i++;
                }
            }

        }

        table.setBorder (0);
        table.setWidth (100,KDReports::Unit::Percent);
        mReport->addElement (table,Qt::AlignJustify);
        //        mReport->addPageBreak ();

        //        if( parti == "Cumhuriyet Halk Partisi" ){
        //            mReport->addPageBreak ();
        //        }

    }
    mReport->addPageBreak ();



}

void BirimListForm::buildLiderler()
{

    std::cout << __LINE__ << " " << __FUNCTION__ << "\n";

    {
        QImage imgLogo;

        if( imgLogo.load ("logoNoOpacity.png") ){


            mReport->addVerticalSpacing (60);

            KDReports::ImageElement element(imgLogo);

            mReport->addElement (element,Qt::AlignmentFlag::AlignCenter);

            mReport->addVerticalSpacing (0.01);

            KDReports::TextElement title(QString("Serik Belediyesi %1\nFaaliyet Raporu").arg (this->selectedYear ()));
            title.setBold (true);
            title.setFontFamily ("Cambria");
            title.setPointSize (16);

            mReport->addElement (title,Qt::AlignmentFlag::AlignCenter);

        }else{
            std::cout << "logoNoOpacity.png Can Not Load\n";
        }
        mReport->addPageBreak ();
    }

    {
        QImage imgLogo;

        if( imgLogo.load ("ataturk.jpg") ){


            KDReports::ImageElement element(imgLogo);

            mReport->addElement (element,Qt::AlignmentFlag::AlignCenter);

            mReport->addVerticalSpacing (0.1);

            KDReports::TextElement title(QString("\"Türk, Öğün, Çalış, Güven.\""));
            title.setBold (true);
            title.setFontFamily ("Cambria");
            title.setPointSize (16);

            mReport->addElement (title,Qt::AlignmentFlag::AlignLeft);

            KDReports::TextElement title1(QString("M. Kemal ATATÜRK"));
            //            title1.setBold (true);
            title1.setItalic (true);
            title1.setFontFamily ("Cambria");
            title1.setPointSize (16);

            mReport->addElement (title1,Qt::AlignmentFlag::AlignRight);

        }
        mReport->addPageBreak ();
    }



    {
        QImage imgLogo;

        if( imgLogo.load ("cumhurbaskani.jpg") ){


            KDReports::ImageElement element(imgLogo);

            mReport->addElement (element,Qt::AlignmentFlag::AlignCenter);

            mReport->addVerticalSpacing (0.01);

            KDReports::TextElement title(QString("Recep Tayyip ERDOĞAN"));
            title.setBold (false);
            title.setFontFamily ("Cambria");
            title.setPointSize (16);

            KDReports::TextElement title1(QString("Türkiye Cumhuriyeti Cumhurbaşkanı"));
            title1.setBold (true);
            title1.setFontFamily ("Cambria");
            title1.setPointSize (16);

            mReport->addElement (title,Qt::AlignmentFlag::AlignCenter);
            mReport->addElement (title1,Qt::AlignmentFlag::AlignCenter);

        }
        mReport->addPageBreak ();
    }


    {
        QImage imgLogo;

        if( imgLogo.load ("baskan.jpg") ){


            KDReports::ImageElement element(imgLogo);

            mReport->addElement (element,Qt::AlignmentFlag::AlignCenter);

            mReport->addVerticalSpacing (0.01);

            KDReports::TextElement title(QString("Enver APUTKAN"));
            title.setBold (false);
            title.setFontFamily ("Cambria");
            title.setPointSize (16);

            KDReports::TextElement title1(QString("Serik Belediye Başkanı"));
            title1.setBold (true);
            title1.setFontFamily ("Cambria");
            title1.setPointSize (16);

            mReport->addElement (title,Qt::AlignmentFlag::AlignCenter);
            mReport->addElement (title1,Qt::AlignmentFlag::AlignCenter);

        }
        mReport->addPageBreak ();
    }


    {

        KDReports::TextElement title1(QString("ÜST YÖNETİCİNİN SUNUŞU"));
        title1.setFontFamily ("Cambria");
        title1.setPointSize (16);
        mReport->addElement (title1,Qt::AlignmentFlag::AlignLeft);
        mReport->addVerticalSpacing (0.1);

        {
            KDReports::TextElement title(QString("\tDeğerli Serikliler,"));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }

        //        {
        //            KDReports::TextElement title(QString("\tDeğerli Çalışma Arkadaşlarım,"));
        //            title.setFontFamily ("Tahoma");
        //            title.setPointSize (12);
        //            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        //        }

        {
            KDReports::TextElement title(QString("\tGöreve geldiğimiz 2019 yılından bugüne şeffaf bir yerel yönetim anlayışı ile "
                                                 "çalışmalarımıza devam ediyoruz. Geride bıraktığımız süreç içerisinde pandeminin yarattığı "
                                                 "olumsuz koşulları ortadan kaldırmak ve ilçemizin süregelen sorunlarına kalıcı çözümler "
                                                 "bulmak için canla başla çalışıp, sahip olduğumuz refah ve huzuru birlikte koruduk ve geliştirdik. "
                                                 "Bölgemizde yaşanan yangın, sel, hortum gibi doğal afetlerde, üzerimize düşen sorumluluğun farkında olup "
                                                 "tüm kaynaklarımızla halkımızın ve komşularımızın yanında olduk."));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        {
            KDReports::TextElement title(QString("\tBir yıllık faaliyet raporumuzu incelediğinizde, başta mali bütçe olmak üzere "
                                                 "tüm faaliyetlerimizi bütün detayları ile göreceksiniz. Önce tasarrufu ilke edindik, ardından da "
                                                 "tüm harcamaları kontrol altına aldık. Arkadaşlarımızın özverili ve planlı çalışmaları ile, "
                                                 "insan kaynaklarını çok iyi kullanarak, az zamanda önemli işleri sonuçlandırmanın mutluluğunu yaşadık. "
                                                 "İlçemizde modern şehircilik anlayışını sergileyeceğimiz projelerimizi hayata geçirmeye devam edeceğiz. "
                                                 "İlk gün söylediğimiz gibi görevde bulunduğumuz süre içerisinde daima \"şeffaf, dürüst, tarafsız, insan odaklı ve "
                                                 "kamu çıkarını esas alan yönetim anlayışımızı\" sürdüreceğimizden hiç kimsenin şüphesi olmasın."));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }

        {
            KDReports::TextElement title(QString("\tBütün bu çalışmalarımızda emeğiyle, özverisiyle bizleri destekleyen çalışma arkadaşlarımıza, "
                                                 "meclis üyelerimize ve her koşulda yanımızda olan siz değerli hemşehrilerimize teşekkür eder, "
                                                 "2021 Yılı Faaliyet Raporumuzu bilgilerinize sunarım."));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        //        {
        //            KDReports::TextElement title(QString("\tBir yıllık faaliyet raporumuzu siz halkımızın olur ve görüşlerine sunarken, çalışmalarımıza destek veren tüm meclis üyesi ve mesai arkadaşlarımıza teşekkür ediyor, sevgi ve saygılarımı sunuyorum."));
        //            title.setFontFamily ("Tahoma");
        //            title.setPointSize (12);
        //            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        mReport->addVerticalSpacing (5);
        //        }

        {
            KDReports::TableElement table;

            table.setHeaderColumnCount (3);

            KDReports::TextElement title(QString("Enver APUTKAN"));
            title.setBold (true);
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            table.cell (0,2).addElement (title,Qt::AlignmentFlag::AlignCenter);

            KDReports::TextElement title2(QString("Serik Belediye Başkanı"));
            title2.setBold (true);
            title2.setFontFamily ("Tahoma");
            title2.setPointSize (12);
            table.cell (1,2).addElement (title2,Qt::AlignmentFlag::AlignCenter);

            table.setWidth (100,KDReports::Unit::Percent);

            table.setBorder (0);

            mReport->addElement (table,Qt::AlignmentFlag::AlignJustify);
        }


        mReport->addPageBreak ();
    }




    {


        mReport->addVerticalSpacing (55);


        KDReports::TextElement title1(QString("İÇ KONTROL GÜVENCE BEYANI"));
        title1.setFontFamily ("Cambria");
        title1.setPointSize (16);
        mReport->addElement (title1,Qt::AlignmentFlag::AlignLeft);
        mReport->addVerticalSpacing (0.1);

        {
            KDReports::TextElement title(QString("\tÜst yönetici olarak yetkim dahilinde;"));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }

        {
            KDReports::TextElement title(QString("\tBu raporda yer alan bilgilerin güvenilir, tam ve doğru olduğunu beyan ederim. Bu raporda açıklanan faaliyetler için bütçe ile tahsis edilmiş kaynakların, planlanmış amaçlar doğrultusunda ve iyi mali yönetim ilkelerine uygun olarak kullanıldığını ve iç kontrol sisteminin işlemlerin yasallık ve düzenliliğine ilişkin yeterli güvenceyi sağladığını bildiririm."));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        {
            KDReports::TextElement title(QString("\tBu güvence, üst yönetici olarak sahip olduğum bilgi ve değerlendirmeler, iç kontroller, iç denetçi raporları ile Sayıştay raporları gibi bilgim dahilindeki hususlara dayanmaktadır. Burada raporlanmayan, idarenin menfaatlerine zarar veren herhangi bir husus hakkında bilgim olmadığını beyan ederim. 31/Aralık/2021"));
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            mReport->addElement (title,Qt::AlignmentFlag::AlignJustify);
        }


        {
            mReport->addVerticalSpacing (5);
            KDReports::TableElement table;

            table.setHeaderColumnCount (3);

            KDReports::TextElement title(QString("Enver APUTKAN"));
            title.setBold (true);
            title.setFontFamily ("Tahoma");
            title.setPointSize (12);
            table.cell (0,2).addElement (title,Qt::AlignmentFlag::AlignCenter);

            KDReports::TextElement title2(QString("Serik Belediye Başkanı"));
            title2.setBold (true);
            title2.setFontFamily ("Tahoma");
            title2.setPointSize (12);
            table.cell (1,2).addElement (title2,Qt::AlignmentFlag::AlignCenter);

            table.setWidth (100,KDReports::Unit::Percent);

            table.setBorder (0);

            mReport->addElement (table,Qt::AlignmentFlag::AlignJustify);
        }


        mReport->addPageBreak ();
    }


}

QVector<QString> BirimListModel::getBirimListResmi()
{

    QVector<QString> list;
    auto filter = document{};

    mongocxx::options::find findOptions;

    auto sortDoc = document{};
    sortDoc.append(kvp("sira",-1));



    filter.append(kvp("sira",make_document(kvp("$gte",bsoncxx::types::b_int32{0}))));

    findOptions.limit (100);
    findOptions.sort (sortDoc.view ());


    try{
        auto cursor = this->getDB()->db()->collection (SerikBLDCore::IK::BirimKey::Collection).find (filter.view (),findOptions);
        for( auto item__ : cursor )
        {
            SerikBLDCore::IK::BirimItem __item;
            __item.setDocumentView (item__);
            list.push_back (__item.birimAdi ());

        }
    }catch(mongocxx::exception &e){
        //        errorOccured(e.what ());
        qDebug() << e.what();
    }


    return list;


}


void BirimListForm::initHeader()
{


    mReport->setTopPageMargin (10);
    mReport->setBottomPageMargin (10);

    {
        KDReports::TableElement tableELement;

        KDReports::TextElement headerElement;
        headerElement.setText (QString("Serik Belediyesi"));
        headerElement.setTextColor (QColor(0,25,55));
        headerElement.setFontFamily ("Arial");
        headerElement.setPointSize (10);

        tableELement.cell(0,0).addElement (headerElement,Qt::AlignLeft);

        {
            KDReports::TextElement headerElement1;
            headerElement1.setText (QString("%1 Faaliyet Raporu").arg (this->selectedYear ()));
            headerElement1.setTextColor (QColor(0,25,55));
            headerElement1.setBold (true);
            headerElement1.setFontFamily ("Arial");
            headerElement1.setPointSize (10);
            tableELement.cell(0,1).addElement (headerElement1,Qt::AlignRight);
        }


        tableELement.setWidth (100,KDReports::Percent);
        tableELement.setBackground (QBrush(QColor(74,215,255,50)));
        tableELement.setBorder (0);
        mReport->header ().addElement (tableELement);
    }



    // Footer information
    {
        KDReports::HLineElement hLineElement;
        hLineElement.setColor (QColor(200,200,200));
        hLineElement.setThickness (1);
        mReport->footer ().addElement (hLineElement);

        KDReports::TableElement tableELement;



        tableELement.cell (0,0).addElement (KDReports::TextElement("http://www.serik.bel.tr/"),Qt::AlignLeft);

        //        {
        //            KDReports::TextElement headerElement1;
        //            headerElement1.setText (QString("%1 Faaliyet Raporu").arg (this->selectedYear ()));
        //            headerElement1.setTextColor (QColor(0,25,55));
        //            headerElement1.setBold (true);
        //            headerElement1.setFontFamily ("Arial");
        //            headerElement1.setPointSize (10);
        //            tableELement.cell(0,1).addElement (headerElement1,Qt::AlignRight);
        //        }
        tableELement.cell (0,1).addElement (KDReports::TextElement("Sayfa "),Qt::AlignRight);
        tableELement.cell (0,0).setRowSpan (1);

        tableELement.cell (0,1).addVariable (KDReports::PageNumber);
        //        tableELement.cell (0,1).addElement (KDReports::TextElement(" / "));
        //        tableELement.cell (0,1).addVariable( KDReports::PageCount );
        tableELement.setWidth (100,KDReports::Unit::Percent);
        tableELement.setBackground (QBrush(QColor(0,236,106,50)));

        tableELement.setBorder (0);
        mReport->footer().addElement ( tableELement, Qt::AlignJustify);

    }



}

void BirimListForm::on_pushButton_SaveList_clicked()
{
    QFile file("list.dat");

    if( file.open (QIODevice::ReadWrite) ){
        QDataStream in(&file);
        in << this->mModel->rowCount () ;
        for( int i = this->mModel->rowCount ()-1 ; i >= 0 ; i-- ){
            in << this->mModel->item (i,0)->text ();
            in << this->mModel->item (i,1)->text ();
            in << this->mModel->item (i,2)->text ();
        }
        file.close ();
    }
}

void BirimListForm::on_pushButton_LoadList_clicked()
{
    QFile file("list.dat");

    if( file.open (QIODevice::ReadOnly) ){

        int row = 0 ;

        QDataStream out(&file);

        out >> row;

        if( row ){
            mModel->removeRows (0,mModel->rowCount ());
        }

        for( int i = 0 ; i < row ; i++ ){

            QString birim;
            QString mudur;
            QString vekil;

            out >> birim;
            out >> mudur;
            out >> vekil;

            mModel->insertRow (0,new QStandardItem(birim));
            mModel->setItem (0,1,new QStandardItem(mudur));
            mModel->setItem (0,2,new QStandardItem(vekil));

        }
    }
}
