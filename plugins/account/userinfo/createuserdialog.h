#ifndef CREATEUSERDIALOG_H
#define CREATEUSERDIALOG_H

#include <QDialog>
#include <QProcess>

#include "../../pluginsComponent/switchbutton.h"

namespace Ui {
class CreateUserDialog;
}

class CreateUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateUserDialog(QStringList userlist, QWidget *parent = 0);
    ~CreateUserDialog();

    void set_face_label(QString iconfile);

    void confirm_btn_status_refresh();
    void tiplabel_status_refresh();

private:
    Ui::CreateUserDialog *ui;

    SwitchButton * autologinSwitchBtn;

    int ostype;

    QProcess * process;
    bool back;
    QStringList usersStringList;

private slots:
    void confirm_btn_clicked_slot();
    void name_legality_check_slot(QString username);
    void pwd_legality_check_slot(QString pwd);
    void pwdsure_legality_check_slot(QString pwdsure);
    void pin_legality_check_slot(QString pin);
    void pinsure_legality_check_slot(QString pinsure);

    void name_conflict_group_slot();

Q_SIGNALS:
    void user_info_send(QString username, QString pwd, QString pin, int atype, bool autologin);
};

#endif // CREATEUSERDIALOG_H
