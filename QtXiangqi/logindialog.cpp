#include "logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent)
{
    ui_.setupUi(this);
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui_.retranslateUi(this);
        break;
    default:
        break;
    }
}

void LoginDialog::on_usernameEdit_editingFinished()
{
    username_ = ui_.usernameEdit->text();
}

void LoginDialog::on_passwordEdit_editingFinished()
{
    password_ = ui_.passwordEdit->text();
}
