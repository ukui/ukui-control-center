#ifndef CHANGEPWDDIALOG_H
#define CHANGEPWDDIALOG_H

#include <QDialog>

namespace Ui {
class ChangePwdDialog;
}

class ChangePwdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePwdDialog(QWidget *parent = 0);
    ~ChangePwdDialog();

    void set_faceLabel(QString iconfile);
    void set_usernameLabel(QString username);
    void set_pwd_type(QString type);
    void confirmbtn_status_refresh();

private:
    Ui::ChangePwdDialog *ui;

public slots:
    void confirm_slot();
    void legality_check_slot(QString text);
    void make_sure_slot(QString text);

Q_SIGNALS:
    void passwd_send(QString pwd, QString username);
};

#endif // CHANGEPWDDIALOG_H
