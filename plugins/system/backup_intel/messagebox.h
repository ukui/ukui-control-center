#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QDialog>
#include <QPainter>
#include <QProcess>
#include <QPainterPath>
#include <QtDBus>
#include "ui_messageboxdialog.h"
namespace Ui {
class MessageBox;
}

class MessageBox : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox();

private:
    Ui::MessageBox *ui;
    void paintEvent(QPaintEvent *event);
    void initUi();
    QDBusInterface *m_rebootInterface;
};

#endif // MESSAGEBOX_H
