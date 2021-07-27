#ifndef MESSAGEBOXDIALOG_H
#define MESSAGEBOXDIALOG_H
#include <QDialog>
#include <QPainter>
#include <QProcess>
#include <QPainterPath>
#include <QtDBus>
#include "ui_messageboxdialog.h"

#ifdef __cplusplus
extern "C" {
#include "clib-syslog.h"
}
#endif

namespace Ui {
class Dialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageDialog(QWidget *parent = nullptr);
    ~MessageDialog();
private:
    Ui::Dialog *ui;
    void paintEvent(QPaintEvent *event);
    void initUi();
    QDBusInterface *m_areaInterface;
};

#endif // MESSAGEBOXDIALOG_H
