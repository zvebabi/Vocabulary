#ifndef TABLE_H
#define TABLE_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QUrl>
#include <QUrlQuery>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QStringList>
#include <QRegExp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDesktopWidget>

typedef QMap<QString, int> Dict_type;
typedef Dict_type::const_iterator Iter;
typedef QVector<Iter> My_vector;
typedef My_vector::const_iterator Iter_vect;

const int FREQ = 3; //frequency of used words

namespace Ui {
class Table;
}

class Table : public QWidget
{
    Q_OBJECT

public:
    explicit Table(QWidget *parent = 0);
    QByteArray GET(QUrl);
    ~Table();
public slots:
    void show_table(My_vector sw);
    void getLang();
private slots:
    void on_TR_BTN_clicked();
    void on_CSV_BTN_clicked();
    void on_label_linkActivated(const QString &link);

private:
    Ui::Table *ui;
    QTableWidgetItem *protoitem = new QTableWidgetItem;
    void moveToCenter();

};

#endif // TABLE_H
