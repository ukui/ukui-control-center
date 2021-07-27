#ifndef DELAUTOBOOT_H
#define DELAUTOBOOT_H

#include <QDialog>
#include <QString>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

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
class DelAutoBoot;
}

class DelAutoBoot : public QDialog
{
    Q_OBJECT

public:
    explicit DelAutoBoot(QString appname, QString bname, QPixmap pixmap, QWidget *parent = 0);
    ~DelAutoBoot();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::DelAutoBoot *ui;

private:
    QString app_name;
    QString b_name;

    void initUI();

Q_SIGNALS:
    void autoboot_deleted(QString name, QString bname);
};
#endif // DELAUTOBOOT_H
