#ifndef ADDSHORTCUT_H
#define ADDSHORTCUT_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class AddShortcut;
}

class AddShortcut : public QDialog
{
    Q_OBJECT

public:
    explicit AddShortcut(QWidget *parent = 0);
    ~AddShortcut();

    void update_dialog_set(QString gsettings_path, QString name, QString exec);
    void update_dialog_unset();

    void refresh_certainbtn_status();

private:
    Ui::AddShortcut *ui;

    QString gspath;

private slots:
    void open_filedialog_slot();

Q_SIGNALS:
    void program_info_signal(QString path, QString name, QString exec);
};

#endif // ADDSHORTCUT_H
