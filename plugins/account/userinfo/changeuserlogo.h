#ifndef CHANGEUSERLOGO_H
#define CHANGEUSERLOGO_H

#include <QDialog>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QFrame;
class QButtonGroup;
class FlowLayout;
class QPixmap;
class QDBusInterface;

class ChangeUserLogo : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserLogo(QString n, QString op, QWidget *parent = nullptr);
    ~ChangeUserLogo();

public:
    void initUI();
    void loadSystemLogo();
    void requireUserInfo(QString logo, QString type);
    void refreshUserLogo(QString logo);
    void setupConnect();

public:
    QVBoxLayout * culMainVerLayout;
    QHBoxLayout * culUserHorLayout;
    QVBoxLayout * culUserInfoVerLayout;
    QHBoxLayout * culLogoNoteHorLayout;
    QHBoxLayout * culMoreLogoHorLayout;
    QHBoxLayout * culBottomBtnsHorLayout;

    QLabel * culNickNameLabel;
    QLabel * culLogoLabel;
    QLabel * culTypeLabel;
    QLabel * culNoteLabel;

    QPushButton * culMoreLogoBtn;
    QPushButton * culCancelBtn;
    QPushButton * culConfirmBtn;

    QButtonGroup * logosBtnGroup;

    QFrame * logosFrame;

    FlowLayout * logosFlowLayout;

private:
    QString name;
    QString selected;

    QDBusInterface * culiface;

private:
    QPixmap makeRoundLogo(QString logo, int wsize, int hsize, int radius);
    bool setCulTextDynamic(QLabel * label, QString string);

};

#endif // CHANGEUSERLOGO_H
