#include "logindialog.h"

LoginDialog::LoginDialog(QWidget* parent,
                         const QString& defaultPid,
                         const QString& defaultPassword) :
    QDialog(parent)
{
    ui_.setupUi(this);
    setWindowTitle(tr("Login"));

    pid_ = defaultPid;
    password_ = defaultPassword;

    ui_.pidEdit->setText(pid_);
    ui_.passwordEdit->setText(password_);
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

void LoginDialog::on_pidEdit_editingFinished()
{
    pid_ = ui_.pidEdit->text();
}

void LoginDialog::on_passwordEdit_editingFinished()
{
    password_ = ui_.passwordEdit->text();
}
