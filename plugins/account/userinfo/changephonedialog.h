#ifndef CHANGEPHONEDIALOG_H
#define CHANGEPHONEDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QStackedWidget>
#include "QGSettings/QGSettings"
#include "digitalauthdialog.h"
#include "phoneauthdialog.h"
#include "digitalphonedialog.h"
#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"


class QLabel;
class QPushButton;

namespace Ui {
class ChangePhoneDialog;
}


class ChangePhoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePhoneDialog(QString username, QWidget *parent = nullptr);
    ~ChangePhoneDialog();

    void initUI(QString username);
    void initDbus();
    void initConnect();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::ChangePhoneDialog *ui;

    int countdown;
    QLabel *title_label;
    QLabel * getVerifiedCodeLabel;
    QPushButton *closeBtn;
    QStackedWidget  *m_pStackedWidget;
    DigitalAuthDialog *m_pdigitalAuthWidget;
    PhoneAuthDialog *m_pPhoneAuthWidget;
    DigitalPhoneDialog *m_pdigitalPhoneWidget;
    QGSettings  *m_style = nullptr;

    QAction *phonePicture;
    QAction *smsPicture;

    QDBusInterface *m_interface1;
    QDBusInterface *m_interface2;

    QTimer *start_timer;

    QGSettings *m_wifi;

    bool m_isNightMode;
    bool oldphonestatus;
    bool phonechangestatus;
    bool phonestatus;
    bool codestatus;
    bool phoneNumChangeSuccess;
    QString oldphone;
    QString oldcode;
    QString m_username;
    QString temptoken;

private slots:
    void setphonestyle(QString key);
    void slotGetVerifyCode(bool clicked);
    void slotSubmitClicked(bool clicked);

};

#endif // CHANGEPHONEDIALOD_H
