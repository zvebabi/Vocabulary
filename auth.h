#ifndef AUTH_H
#define AUTH_H

#include <QWidget>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QDesktopWidget>

namespace Ui {
class Auth;
}

class Auth : public QWidget
{
    Q_OBJECT

signals:
    void auth_success(QString new_token);

public slots:
    void url_Changed(QUrl);

public:
    explicit Auth(QWidget *parent = 0);
    ~Auth();

private:
    Ui::Auth *ui;
    QString token;
    void moveToCenter();

};

#endif // AUTH_H
