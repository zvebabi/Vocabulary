#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //Qstring path = QFileInfo(argv[0]).dir().path();

    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Vocabulary 1.0");
    w.show();

    return a.exec();
}
