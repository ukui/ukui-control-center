#ifndef CHANGEPASSWD_H
#define CHANGEPASSWD_H

#include <QWidget>
#include <QDialog>
#include <QDebug>
#include <QStackedWidget>
#include <QPainter>
#include <QPainterPath>
#include "QGSettings/QGSettings"
#include "digitalauthdialog.h"
#include "phoneauthdialog.h"
#include "changepwddialog.h"
#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"

class QLabel;
class QPushButton;


class changepasswd : public QDialog
{
    Q_OBJECT
public:
    explicit changepasswd(QString username, QWidget *parent = nullptr);
    ~changepasswd();

    void initUI(QString username);
    void setQSS();

Q_SIGNALS:
    void newpdSignal();
    void changepwd();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *title_label;
    QPushButton *closeBtn;
    QStackedWidget  *m_pStackedWidget;
    DigitalAuthDialog *m_pdigitalAuthWidget;
    PhoneAuthDialog *m_pPhoneAuthWidget;
    ChangePwdDialog *m_pChangePwdDialog;
    QGSettings  *m_style = nullptr;
private slots:
    void setpwdstyle(QString key);
};

#endif // CHANGEPASSWD_H
