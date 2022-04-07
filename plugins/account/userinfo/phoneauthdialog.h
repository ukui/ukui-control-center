#ifndef PHONEAUTHDIALOG_H
#define PHONEAUTHDIALOG_H

#include <QWidget>
#include <QTimerEvent>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLabel>
#include <QRegExpValidator>
#include <QEventLoop>
#include <QGSettings/QGSettings>
#include <QTimer>

class QAction;
class QLineEdit;
class QPushButton;
class QStackedWidget;

class PhoneAuthDialog : public QWidget
{
    Q_OBJECT
public:
    explicit PhoneAuthDialog(QString username,QWidget *parent = nullptr);
    ~PhoneAuthDialog();
    void initUI();
    void setQSS();
    void hidePromptMsg();
    void showPromptMsg();
    void hideQRPromptMsg();
    void showQRPromptMsg();
public slots:
    void QRStatusChangedSlots(QString name,QString passwd,int status);
    void themeChanged(const quint32 currentTheme);
    void getCodeChanged();

Q_SIGNALS:
    void returnSignal();
    void confirmSignal();
    void getCodeChange();
private:
    int daojishi;
    QTimer *m_qrTimeout;
    QString m_username;
    QLabel *m_pPromptMessage;
    QLabel *m_pQRPromptMessage;
    QLabel *m_qr;
    QStackedWidget *stackWidget;

    QAction *phonePicture;
    QAction *smsPicture;

    QPushButton * wechatAuthBtn;
    QPushButton * phoneAuthBtn;
    QLineEdit *phoneNumLine;
    QLineEdit *verifyCodeLine;
    QPushButton * getVerifyCodeBtn;

    QDBusInterface *m_interface1;
    QDBusInterface *m_interface2;
    QDBusInterface *m_interface3;
    QDBusInterface *m_interface4;

    QString m_phone;

    QGSettings *m_wifi;
    QPushButton *returnButton;
    QPushButton *confirmButton;

    void getQRCodeFromURL(QPixmap &qrcode);
    QPixmap beautifyQRCode(QPixmap &);

    bool phonestatus;
    bool codestatus;
    bool qrstatus = false;
    bool is_phoneVerifyChecked;
    bool is_nightTheme;
};

#endif // PHONEAUTHDIALOG_H
