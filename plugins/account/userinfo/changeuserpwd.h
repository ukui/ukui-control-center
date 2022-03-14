#ifndef CHANGEUSERPWD_H
#define CHANGEUSERPWD_H

#include <QDialog>

#include "pwdcheckthread.h"
#include "passwdcheckutil.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}

#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2

#endif

class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QMouseEvent;
class QKeyEvent;

class ChangeUserPwd : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserPwd(QString n, QWidget *parent = nullptr);
    ~ChangeUserPwd();

public:
    void initUI();
    void setupConnect();
    void setupStatus(QString n);

    void makeSurePwqualityEnabled();

    void updateTipLableInfo(QLabel *label, QString info);
    void refreshConfirmBtnStatus();
    void refreshCloseBtnStatus();
public:
    QVBoxLayout * mainVerLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * currentPwdHorLayout;
    QHBoxLayout * newPwdHorLayout;
    QHBoxLayout * tipHorLayout;
    QVBoxLayout * surePwdWithTipVerLayout;
    QHBoxLayout * surePwdHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;
    QHBoxLayout *curTipHorLayout;
    QHBoxLayout *newTipHorLayout;
    QHBoxLayout *sureTipHorLayout;

    QPushButton * cancelBtn;
    QPushButton * confirmBtn;

    QLabel *currentPwdLabel;
    QLabel *curTipLabel;
    QLabel *newPwdLabel;
    QLabel *newTipLabel;
    QLabel *surePwdLabel;
    QLabel *tipLabel;

    QLineEdit * currentPwdLineEdit;
    QLineEdit * newPwdLineEdit;
    QLineEdit * surePwdLineEdit;

    QPushButton *currentPwdEyeBtn;
    QPushButton *newPwdEyeBtn;
    QPushButton *surePwdEyeBtn;

protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *target, QEvent *event);
private:
    QString name;
    QString curPwdTip;
    QString newPwdTip;
    QString surePwdTip;

    bool isCurrentUser;
    bool isChecking;
    bool enablePwdQuality;

    PwdCheckThread * thread1ForCheckPwd;

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

private:
    bool setTextDynamicInPwd(QLabel * label, QString string);

    void checkPwdLegality();
    bool isContainLegitimacyChar(QString word);

private slots:
    void test();

};

#endif // CHANGEUSERPWD_H
