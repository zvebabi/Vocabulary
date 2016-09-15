#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QMap>
#include <QDir>
#include "table.h"
#include "auth.h"

const int MAX_COUNT = 200; //messages count(max 200)

typedef QMap<QString, int> Dict_type;
typedef Dict_type::const_iterator Iter;
typedef QVector<Iter> My_vector;
typedef My_vector::const_iterator Iter_vect;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


signals:
    void get_mess_success(QString fn);
    void sort_loading_success(My_vector sw);

public slots:
    void get_messages(QString token);
    void analize_word(QString fn);

private slots:
    void on_AUTH_BTN_clicked();
    void on_GET_WORDS_clicked();

private:
    Ui::MainWindow *ui;
    QByteArray GET(QUrl u)
    {
        Table table;
        return table.GET(u);
    }
    QByteArray query(QString token, int k);
    QString create_file(QJsonArray items);
    Dict_type words;
    My_vector sorted_words;
    void moveToCenter();


};

#endif // MAINWINDOW_H
