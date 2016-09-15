#include "auth.h"
#include "ui_auth.h"

Auth::Auth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Auth)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    moveToCenter();
    ui->WEB->load(QUrl("https://oauth.vk.com/authorize?client_id=4661535&scope=messages&redirect_uri=https://oauth.vk.com/blank.html&display=mobile&v=5.27&response_type=token"));
    connect(ui->WEB, SIGNAL(urlChanged(QUrl)), this, SLOT(url_Changed(QUrl)));

}

Auth::~Auth()
{
    delete ui;
}

void Auth::moveToCenter()
{
    QRect rect = frameGeometry();
    rect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(rect.topLeft());
}

void Auth::url_Changed(QUrl u)
{
    if(!u.toString().contains("access_token"))
        return;

    u=u.toString().replace("#","?");
    token = QUrlQuery(u).queryItemValue("access_token");
    emit auth_success(token);
}
