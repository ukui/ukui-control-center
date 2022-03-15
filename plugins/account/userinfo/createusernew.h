#ifndef CREATEUSERNEW_H
#define CREATEUSERNEW_H

#include <QDialog>

#include "passwdcheckutil.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}
#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2
#endif

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QRadioButton;
class QFrame;
class QLineEdit;
class QPushButton;
class QButtonGroup;

class CreateUserNew : public QDialog
{
    Q_OBJECT

public:
    explicit CreateUserNew(QStringList allUsers, QWidget *parent = nullptr);
    ~CreateUserNew();

public:
    void initUI();
    void setConnect();
    void refreshConfirmBtnStatus();
    void makeSurePwdNeedCheck();
    void nameLegalityCheck(QString username);
    void nameLegalityCheck2(QString nickname);
    void pwdLegalityCheck(QString pwd);

public:
    QVBoxLayout * mainVerLayout;
    QHBoxLayout * usernameHorLayout;
    QHBoxLayout * nicknameHorLayout;
    QHBoxLayout *hostnameHorLayout;
    QHBoxLayout * newPwdHorLayout;
    QHBoxLayout * surePwdHorLayout;
    QHBoxLayout * tipHorLayout;
    QHBoxLayout * typeNoteHorLayout;
    QHBoxLayout * adminHorLayout;
    QVBoxLayout * admin1VerLayout;
    QVBoxLayout * admin2VerLayout;
    QHBoxLayout * standardHorLayout;
    QVBoxLayout * standard1VerLayout;
    QVBoxLayout * standard2VerLayout;
    QHBoxLayout * bottomHorLayout;
    QHBoxLayout *usernameTipHorLayout;
    QHBoxLayout *nicknameTipHorLayout;
    QHBoxLayout *hostnameTipHorLayout;
    QHBoxLayout *newPwdTipHorLayout;

    QLabel * usernameLabel;
    QLabel * nicknameLabel;
    QLabel *hostnameLabel;
    QLabel * newPwdLabel;
    QLabel * surePwdLabel;
    QLabel * tipLabel;
    QLabel * typeNoteLabel;
    QLabel * adminLabel;
    QLabel * adminDetailLabel;
    QLabel * standardLabel;
    QLabel * standardDetailLabel;
    QLabel *usernameTipLabel;
    QLabel *nicknameTipLabel;
    QLabel *hostnameTipLabel;
    QLabel *newpwdTipLabel;


    QLineEdit * usernameLineEdit;
    QLineEdit * nicknameLineEdit;
    QLineEdit *hostnameLineEdit;
    QLineEdit * newPwdLineEdit;
    QLineEdit * surePwdLineEdit;

    QFrame * adminFrame;
    QFrame * standardFrame;

    QRadioButton * adminRadioBtn;
    QRadioButton * standardRadioBtn;

    QPushButton * cancelBtn;
    QPushButton * confirmBtn;

    QButtonGroup * typeBtnGroup;

public:
    QString userNameTip;
    QString nickNameTip;
    QString newPwdTip;
    QString surePwdTip;
    QString oldName;
    QString oldNickName;

    QStringList _allNames;

    bool enablePwdQuality;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *);
    bool setCunTextDynamic(QLabel * label, QString string);

private:
    bool nameTraverse(QString username);
    bool isHomeUserExists(QString username);
    bool checkCharLegitimacy(QString password);

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

};

#endif // CREATEUSERNEW_H
