#ifndef ADDAUTOBOOT_H
#define ADDAUTOBOOT_H

#include <QDialog>
#include <QFileDialog>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

namespace Ui {
class AddAutoBoot;
}

class AddAutoBoot : public QDialog
{
    Q_OBJECT

public:
    explicit AddAutoBoot(QWidget *parent = 0);
    ~AddAutoBoot();

private:
    Ui::AddAutoBoot *ui;

private slots:
    void open_desktop_dir_slots();

Q_SIGNALS:
    void autoboot_adding_signals(QString name, QString exec, QString comment);
};

#endif // ADDAUTOBOOT_H
