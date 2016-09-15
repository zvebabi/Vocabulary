#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setTextVisible(false);
    ui->progressBar->setVisible(false);
    connect(this, SIGNAL(get_mess_success(QString)),this, SLOT(analize_word(QString)));
    moveToCenter();

}

MainWindow::~MainWindow()
{
    delete ui;
}


/*
 * Get messages from vk.com to temp.txt(in temp directory)
 */
void MainWindow::get_messages(QString token)
{
//remove temp file if non empty and clean dictionary
    QString temp = QDir::toNativeSeparators(QDir::tempPath() + QDir::separator() + QString("temp_abra_cadabra.txt"));
    QFile file(temp);
    if(file.exists())
        file.remove();
    sorted_words.clear();
    words.clear();

//ui setting
    ui->AUTH_BTN->setEnabled(false);
    ui->progressBar->setFormat("Loading...");
    ui->progressBar->setVisible(true);
    ui->progressBar->setTextVisible(true);

//check the number of messages
    QByteArray answer = query(token, 0);
    int count = QJsonDocument::fromJson(answer).object()["response"].toObject()["count"].toInt();
    qDebug() << count;
    if(count == 0)
    {
        qDebug() << "No messages!";
        exit(-5);
    }
    int offset_iter = count/MAX_COUNT;

//load all messages
    QTime timer;
    QString filename;

//ui setting
    ui->progressBar->setMaximum(offset_iter);

//start load-loop
    for(int c=0; c<offset_iter; ++c)
    {
        timer.start();
        QByteArray answer = query(token, c);
        QJsonArray items = QJsonDocument::fromJson(answer).object()["response"].toObject()["items"].toArray();

        if(items.isEmpty())     //if empty -> parse error -> if no error, than end of messages
        {
            QJsonObject object = QJsonDocument::fromJson(answer).object()["error"].toObject();
            QString error_msg = object["error_msg"].toString();
            int error_code = object["error_code"].toInt();
            if(!error_msg.isEmpty()) //if no error, end of list
            {
                qDebug() << "Error code is" << error_code;
                qDebug() << "Error messages is ["+ error_msg + "]";
                exit(-4);
            }
            else
            {
                qDebug() << "Job done!";
                break;
            }
        }

        filename = create_file(items);

        ui->progressBar->setValue(c+1);

//make a dynamic latency, because VK API works good with only 3 query per second
        while (timer.elapsed() < 400)  {}

    } //end load loop

    ui->progressBar->setValue(offset_iter);
    ui->progressBar->setFormat("Done!");

    emit get_mess_success(filename);

}
/*
 * Query to VK API.
 * Load all messages
 */
QByteArray MainWindow::query(QString token, int k)
{
    QUrlQuery q;
    q.addQueryItem("out", "1");                                 // 1 - outgoing messages, 0 - incoming messages
    q.addQueryItem("count", QString("%1").arg(MAX_COUNT));      //messages count(max 200)
    q.addQueryItem("offset",QString("%1").arg(k*MAX_COUNT));
    //q.addQueryItem("time_offset","63072000");                   //63072000 sec in 2 years
    q.addQueryItem("v", "5.27");                                //version of Vk Api
    q.addQueryItem("access_token", token);
    QUrl url("https://api.vk.com/method/messages.get");
    url.setQuery(q);
    QByteArray answer = GET(url);
    if(answer.isEmpty())
    {
        qDebug() << "Empty response from message.get";
        exit(-1);
    }
    return answer;
}

/*
 * Create a temp files
 */
QString MainWindow::create_file(QJsonArray items)
{
    QString temp = QDir::toNativeSeparators(QDir::tempPath() + QDir::separator() + QString("temp_abra_cadabra.txt"));
    qDebug() << "write temp in " + temp;
    QFile file(temp);
    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        QRegExp val("[!?\",.:;()#@$%&*\\/\'A-z\\d\\-\\=\\+]");
        for(QJsonArray::Iterator itr = items.begin(); itr != items.end(); itr++)
        {
            QString body = items[itr.i].toObject()["body"].toString().toLower();
            if(body != NULL)
            {
                //delete nonalpha characters and convert space to newline symbol;
                body.remove(val).replace(" ","\n");
                out << body << endl;
            }
        }
        file.close();
    }
    else
    {
        qDebug() << "Can't create a temp.txt";
        exit(-2);
    }
    return temp;
}

void MainWindow::moveToCenter()
{
    QRect rect = frameGeometry();
    rect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(rect.topLeft());
}

/*
 * Implement of analyzing of words.
 * Make a table sorted by popularity of words.
 */
void MainWindow::analize_word(QString fn)
{
//ui setting
    ui->progressBar->setFormat("Analyzing...");
    QFile file(fn);
    qint64 size = file.size();
    ui->progressBar->setMaximum(size);

    qDebug() << fn << "\nanalyze word";

//Reading from file
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        int value = 0; //for progress bar
        while(!file.atEnd())
        {
            QString str = file.readLine();
            value+=str.size();
            ui->progressBar->setValue(value);
            if(str.size()<=3) // delet words less than 3 character from statistic
                continue;
            if(words.find(str) != words.end())
            {
                int i = words.value(str);   // increment repeat words
                ++i;                        //
                words.insert(str,i);
            }
            else
            {
                words.insert(str,1);
            }
        }
        file.close(); //closing file after loading into QMap
        file.remove();//deleting file

//Sorting QMap like Qvector iterator
        Iter i = words.cbegin();
        while(i != words.cend())
            sorted_words.append(i++);
        qSort(sorted_words.begin(), sorted_words.end(), [] (Iter i0, Iter i1) {
            return i0.value() > i1.value();
        });
    }
    else
    {
        qDebug() << "Can't open file to read!";
        exit(-3);
    }

//ui setting
    QApplication::alert(this,0);
    ui->progressBar->setValue(size);
    ui->progressBar->setFormat("Done!");
    ui->GET_WORDS->setEnabled(true);
    ui->AUTH_BTN->setEnabled(true);
    return;
}

/*
 * Authorization on VK.com
 */
void MainWindow::on_AUTH_BTN_clicked()
{
    Auth* auth = new Auth();  //define new window for authorization in Vk
    connect(auth, SIGNAL(auth_success(QString)), auth, SLOT(close()));
    connect(auth, SIGNAL(auth_success(QString)), this, SLOT(get_messages(QString)));
    auth->setWindowTitle("Authorization");
    auth->show();
}

/*
 * Open window with table of words for translating
 */
void MainWindow::on_GET_WORDS_clicked()
{
    Table* table = new Table();
    connect(this, SIGNAL(sort_loading_success(My_vector)), table, SLOT(show_table(My_vector)));
    emit sort_loading_success(sorted_words);
    table->setWindowTitle("My Vocabulary");
    table->show();
}
