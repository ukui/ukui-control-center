#ifndef CHANGETYPEDIALOG_H
#define CHANGETYPEDIALOG_H

#include <QDialog>

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

    int currenttype;

private slots:
    void radioBtn_clicked_slot(int id);
    void confirm_slot();

Q_SIGNALS:
    void type_send(int type, QString username);
};

#endif // CHANGETYPEDIALOG_H
