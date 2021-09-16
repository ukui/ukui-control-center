#ifndef CHANGEUSERTYPE_H
#define CHANGEUSERTYPE_H

#include <QDialog>

class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QFrame;

class QDBusInterface;

class ChangeUserType : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserType(QString objpath, QWidget *parent = nullptr);
    ~ChangeUserType();

public:
    void requireUserInfo(QString logo, QString nname, int id, QString utype);
    void initUI();
    void setConnect();

public:
    QVBoxLayout * cutMainVerLayout;
    QHBoxLayout * cutUserHorLayout;
    QVBoxLayout * cutUserInfoVerLayout;
    QVBoxLayout * cutAdminVerLayout;
    QVBoxLayout * cutStandardVerLayout;
    QHBoxLayout * cutAdminInfoHorLayout;
    QHBoxLayout * cutStandardInfoHorLayout;
    QHBoxLayout * cutAdminInfo2HorLayout;
    QHBoxLayout * cutStandardInfo2HorLayout;
    QHBoxLayout * cutBtnGroupsHorLayout;

    QRadioButton * cutAdminRadioBtn;
    QRadioButton * cutStandardRadioBtn;

    QButtonGroup * cutTypesBtnGroup;

    QLabel * cutNickNameLabel;
    QLabel * cutUserTypeLabel;
    QLabel * cutNoteLabel;
    QLabel * cutAdminLabel;
    QLabel * cutStandardLabel;
    QLabel * cutAdminNoteLabel;
    QLabel * cutStandardNoteLabel;

    QPushButton * cutUserLogoBtn;
    QPushButton * cutConfirmBtn;
    QPushButton * cutCancelBtn;

    QFrame * cutAdminFrame;
    QFrame * cutStandardFrame;

protected:
    bool setTextDynamic(QLabel *label, QString string);

private:
    QString _objpath;
    QString username;
    QString nickname;
    int type;
    QString logo;

    QDBusInterface * cutiface;

};

#endif // CHANGEUSERTYPE_H
