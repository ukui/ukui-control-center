#ifndef CHANGEFACEDIALOG_H
#define CHANGEFACEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QDir>
#include <QListWidgetItem>

namespace Ui {
class ChangeFaceDialog;
}

class ChangeFaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeFaceDialog(QWidget *parent = 0);
    ~ChangeFaceDialog();

    void set_face_label(QString iconfile);
    void set_username_label(QString username);
    void set_account_type_label(QString atype);
    void set_face_list_status(QString facefile);

    void show_faces();

    QMap<QString, QListWidgetItem *> delitemMap;

private:
    Ui::ChangeFaceDialog *ui;

private slots:
    void item_changed_slot(QListWidgetItem * current, QListWidgetItem * previous);

Q_SIGNALS:
    void face_file_send(QString file, QString username);
};

#endif // CHANGEFACEDIALOG_H
