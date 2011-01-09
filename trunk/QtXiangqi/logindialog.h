#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>
#include "ui_logindialog.h"

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent,
                         const QString& defaultPid,
                         const QString& defaultPassword);

    QString getPid() const { return pid_; }
    QString getPassword() const { return password_; }

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_pidEdit_editingFinished();
    void on_passwordEdit_editingFinished();

private:
    Ui::LoginDialog ui_;
    QString         pid_;
    QString         password_;
};

#endif // LOGINDIALOG_H
