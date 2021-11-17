#ifndef CHANGEPHONEINTELDIALOG_H
#define CHANGEPHONEINTELDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QStackedWidget>
#include <QPainterPath>
#include "QGSettings/QGSettings"
#include "digitalauthinteldialog.h"
#include "phoneauthinteldialog.h"
#include "digitalphoneinteldialog.h"
#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"


class QLabel;
class QPushButton;

namespace Ui {
class ChangePhoneIntelDialog;
}


class ChangePhoneIntelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePhoneIntelDialog(QString username, QWidget *parent = nullptr);
    ~ChangePhoneIntelDialog();

    void initUI(QString username);
    void initDbus();
    void initConnect();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::ChangePhoneIntelDialog *ui;

    int countdown;
    QLabel *title_label;
    QLabel * getVerifiedCodeLabel;
    QPushButton *closeBtn;
    QStackedWidget  *m_pStackedWidget;
    DigitalAuthIntelDialog *m_pdigitalAuthWidget;
    PhoneAuthIntelDialog *m_pPhoneAuthWidget;
    DigitalPhoneIntelDialog *m_pdigitalPhoneWidget;
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
