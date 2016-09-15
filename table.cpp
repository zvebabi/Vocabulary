#include "table.h"
#include "ui_table.h"

Table::Table(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Table)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    moveToCenter();

    QStringList labels;
    labels << "Rating" << "Word" << "Translate" << "Example" << "Translation\nexample";

    ui->TABLE->clear();
    ui->TABLE->setWordWrap(true);
    ui->TABLE->setColumnWidth(0,60);
    ui->TABLE->setHorizontalHeaderLabels(labels);
    ui->TABLE->setColumnCount(5);

    protoitem->setTextAlignment(Qt::AlignCenter);
    ui->progressBar->setVisible(false);
    ui->progressBar->setTextVisible(false);
}

Table::~Table()
{
    delete ui;
}

/*
 *  Show table with statistic and non-translated words
 */
void Table::show_table(My_vector sw)
{
    qDebug() <<sw.size();

    float index = sw.first().value();

    int k=0;
    for(Iter_vect i= sw.cbegin(); i != sw.cend(); ++i)
    {
        if(i->value() <FREQ)  // exclude rarely used words
            continue;
        QString l = QString("%1").arg((double)i->value()/index*100, 0, 'f', 2);
        QString w = i->key();
        w.replace(QRegExp("[ \n]"),"");
        QTableWidgetItem *popularity = protoitem->clone();
        popularity->setText(l);
        QTableWidgetItem *word = protoitem->clone();
        word->setText(w);
        ui->TABLE->insertRow(k);
        ui->TABLE->setRowHeight(k,35);   //set row height
//insert words in table
        ui->TABLE->setItem(k,0, popularity);
        ui->TABLE->setItem(k,1, word);

        k++;
    }
    getLang();
    ui->TR_BTN->setEnabled(true);
}

/*
 * List of lang for translate
 */
void Table::getLang()
{
    QUrlQuery q;
    q.addQueryItem("key","dict.1.1.20141211T092706Z.a602156126cf7334.5e2c26ccec215b6ad70d5adb63efdd68906b331d");
    //spare key
    //q.addQueryItem("key", "dict.1.1.20141211T092450Z.b77af3e5833dc722.3d93316d0387bbd88c114147068b8697e911b882");
    QUrl url("https://dictionary.yandex.net/api/v1/dicservice.json/getLangs");
    url.setQuery(q);
    QByteArray answer = GET(url);
    QJsonArray lang = QJsonDocument::fromJson(answer).array();
    QJsonArray ru_lang;

//check for error
    QString error = QJsonDocument::fromJson(answer).object()["message"].toString();
    if(!error.isEmpty())
    {
        qDebug() << "Error: " << error;
        exit(-6);
    }

//select only "from ru" translate direction
    for(QJsonArray::Iterator i=lang.begin(); i!=lang.end(); ++i)
    {
        if(lang[i.i].toString()[0] == 'r')
        {
            if(lang[i.i].toString()[3] == 'e' && lang[i.i].toString()[4] == 'n')
                ru_lang.push_front(lang[i.i]);
            else
                ru_lang.push_back(lang[i.i]);
            ui->LANG_BOX->addItem(lang[i.i].toString().toUpper());
        }
    }
}

/*
 * Implement a query to Dictionary api and update a table
 */
void Table::on_TR_BTN_clicked()
{
    ui->TR_BTN->setEnabled(false);
    ui->CSV_BTN->setEnabled(false);

    int row_count = ui->TABLE->rowCount();

    ui->progressBar->setVisible(true);
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setMaximum(row_count);
    ui->progressBar->setValue(0);

    for (int r = 0; r < row_count; r++)
    {
        QRegExp rx("[ \n]");
        QString word = ui->TABLE->item(r,1)->text().remove(rx);

//Yandex Dictionary API
        QUrlQuery q;
        QString lang = ui->LANG_BOX->currentText().toLower();
        q.addQueryItem("key", "dict.1.1.20141211T092706Z.a602156126cf7334.5e2c26ccec215b6ad70d5adb63efdd68906b331d");
      //spare key
      //q.addQueryItem("key", "dict.1.1.20141211T092450Z.b77af3e5833dc722.3d93316d0387bbd88c114147068b8697e911b882");
        q.addQueryItem("lang",lang);
        q.addQueryItem("text",word);
        QUrl url("https://dictionary.yandex.net/api/v1/dicservice.json/lookup");
        url.setQuery(q);
        QByteArray answer = GET(url);

//check for error
        QString error = QJsonDocument::fromJson(answer).object()["message"].toString();
        if(!error.isEmpty())
        {
            qDebug() << "Error: " << error;
            exit(-6);
        }

//extract translated word
        QJsonObject object = QJsonDocument::fromJson(answer).object()["def"].toArray()[0].toObject()["tr"].toArray()[0].toObject();
        QJsonObject object2 = object["ex"].toArray()[0].toObject();

        QString translated = object["text"].toString();
        QString example = object2["text"].toString();
        QString trExample = object2["tr"].toArray()[0].toObject()["text"].toString();

//if word is wrong
        if(translated.isEmpty())
            translated="Error";
        if(trExample.isEmpty())
            trExample="No Example";
        if(example.isEmpty())
            example="No Example";

//update table
        QTableWidgetItem *pTranslated = protoitem->clone();
        QTableWidgetItem *pTrExample = protoitem->clone();
        QTableWidgetItem *pExample = protoitem->clone();

        pTranslated->setText(translated);
        pTrExample->setText(trExample);
        pExample->setText(example);

        ui->TABLE->setItem(r,2, pTranslated);
        ui->TABLE->setItem(r,3, pTrExample);
        ui->TABLE->setItem(r,4, pExample);

//update progress bar
        ui->progressBar->setValue(r);
    }

//return control to user
    QApplication::alert(this,0);
    ui->TR_BTN->setEnabled(true);
    ui->CSV_BTN->setEnabled(true);

//ui setting
    ui->progressBar->setTextVisible(true);
    ui->progressBar->setValue(row_count);
    ui->progressBar->setFormat("Done!");
}

/*
 * Save into CSV file
 */
void Table::on_CSV_BTN_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Сохранить в:"),QDir::homePath(), tr("Comma separated values (*.csv)"));
    if(filename.isEmpty())
    {
        QMessageBox::information(this, "Alert", "File name is empty!");
        return;
    }
    QFile file(filename);
    QTextStream out(&file);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << filename + " in write mode";

        int lenght = ui->TABLE->rowCount();
        int width = ui->TABLE->columnCount();

        for(int r=0; r< lenght; ++r)
        {
            for(int c=0; c < width; ++c)
            {
                out << ui->TABLE->item(r,c)->text();
                if(c!=width-1) //last column without separator
                {
                    out << ";";
                }
            }
            out << endl;
        }
        file.close();
//"open file" dialog after writing


        QMessageBox::information(this, "Information", "Export finished!");
        /*QMessageBox::StandardButtons answer;
        answer = QMessageBox::question(this, "Export done!", "Open file?", QMessageBox::Yes | QMessageBox::No);
        if(answer == QMessageBox::Yes)
        {
            QDesktopServices::openUrl("file:///"+filename);
        }*/
    }
    else
    {
        qDebug() << "Can't create a " + filename;
        exit(-6);
    }
}

/*
 * Implement of GET query to API
 */
QByteArray Table::GET(QUrl r)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(QNetworkRequest(r));
    QEventLoop wait;
    connect(manager, SIGNAL(finished(QNetworkReply*)), &wait, SLOT(quit()));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));
    QTimer::singleShot(10000, &wait, SLOT(quit()));
    wait.exec();
    QByteArray answer = reply->readAll();
    reply->deleteLater();
    return answer;
}

/*
 * Pay a tribute to Yandex.Dictionary
 */
void Table::on_label_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}

void Table::moveToCenter()
{
    QRect rect = frameGeometry();
    rect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(rect.topLeft());
}
