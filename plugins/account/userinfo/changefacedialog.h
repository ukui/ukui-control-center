#ifndef CHANGEFACEDIALOG_H
#define CHANGEFACEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QDir>
#include <QListWidgetItem>
#include <QFileDialog>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

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
    void custom_face_choosed_slot();

Q_SIGNALS:
    void face_file_send(QString file, QString username);
};

#endif // CHANGEFACEDIALOG_H
