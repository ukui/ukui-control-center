#ifndef CHANGEUSERTYPE_H
#define CHANGEUSERTYPE_H

#include <QDialog>
#include <QWidget>
#include <QApplication>
#include <QMessageBox>
#include <ukcc/widgets/lightlabel.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

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
    QHBoxLayout * tipHorLayout;

    QRadioButton * cutAdminRadioBtn;
    QRadioButton * cutStandardRadioBtn;

    QButtonGroup * cutTypesBtnGroup;

    QLabel * cutNickNameLabel;
    LightLabel * cutUserTypeLabel;
    QLabel * cutNoteLabel;
    QLabel * cutAdminLabel;
    QLabel * cutStandardLabel;
    LightLabel * cutAdminNoteLabel;
    LightLabel * cutStandardNoteLabel;
    QLabel * tipLabel;

    QPushButton * cutUserLogoBtn;
    QPushButton * cutConfirmBtn;
    QPushButton * cutCancelBtn;

    QFrame * cutAdminFrame;
    QFrame * cutStandardFrame;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    bool setTextDynamic(QLabel *label, QString string);

private:
    QString _objpath;
    QString username;
    QString nickname;
    int type;
    QString logo;
    int oldid;

    QDBusInterface * cutiface;

};

#endif // CHANGEUSERTYPE_H
