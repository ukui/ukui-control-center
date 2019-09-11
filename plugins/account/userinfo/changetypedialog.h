#ifndef CHANGETYPEDIALOG_H
#define CHANGETYPEDIALOG_H

#include <QDialog>

#include "../../pluginsComponent/switchbutton.h"

namespace Ui {
class ChangeTypeDialog;
}

class ChangeTypeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeTypeDialog(QWidget *parent = 0);
    ~ChangeTypeDialog();

    void set_face_label(QString iconfile);
    void set_username_label(QString username);
    void set_account_type_label(QString atype);
    void set_current_account_type(int id);
    void set_autologin_status(bool status);

private:
    Ui::ChangeTypeDialog *ui;

    SwitchButton * autologinSwitchBtn;

    int currenttype;
    bool currentloginstatus;

private slots:
    void autologin_status_changed_slot(bool status);
    void radioBtn_clicked_slot(int id);
    void confirm_slot();

Q_SIGNALS:
    void type_send(int type, QString username, bool autologin);
};

#endif // CHANGETYPEDIALOG_H
