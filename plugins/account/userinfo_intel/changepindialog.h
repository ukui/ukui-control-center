#ifndef CHANGEPINDIALOG_H
#define CHANGEPINDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QSharedPointer>
#include <QVBoxLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QString>
#include <QGSettings/QGSettings>
#include "digitalauthdialog.h"
#include "phoneauthdialog.h"
#include "changepwddialog.h"
#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"

class QLabel;
class QPushButton;

namespace Ui {
class ChangePinDialog;
}

class ChangePinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePinDialog(QString username, QWidget *parent = nullptr);
    ~ChangePinDialog();
    static int m_isThemeChanged;                                    //主题

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::ChangePinDialog *ui;
    QPushButton *closeBtn;
    DigitalAuthDialog *m_pdigitalAuthWidget;
    PhoneAuthDialog *m_pPhoneAuthWidget;
    ChangePwdDialog *m_pChangePwdDialog;
    QGSettings  *m_style = nullptr;

Q_SIGNALS:
    void EmitThemeChanged(int theme);
    void changepwd();

private slots:
    void setpwdstyle(QString key);
};

#endif // CHANGEPINDIALOG_H
