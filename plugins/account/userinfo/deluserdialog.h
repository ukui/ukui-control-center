#ifndef DELUSERDIALOG_H
#define DELUSERDIALOG_H

#include <QDialog>
#include <QSignalMapper>
#include <QAbstractButton>

namespace Ui {
class DelUserDialog;
}

class DelUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DelUserDialog(QWidget *parent = 0);
    ~DelUserDialog();

    void set_face_label(QString iconfile);
    void set_username_label(QString username);

private:
    Ui::DelUserDialog *ui;

private slots:
    void btn_clicked_slot(QString key);

Q_SIGNALS:
    void removefile_send(bool removefile, QString username);
};

#endif // DELUSERDIALOG_H
