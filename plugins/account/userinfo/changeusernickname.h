#ifndef CHANGEUSERNICKNAME_H
#define CHANGEUSERNICKNAME_H

#include <QDialog>

class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QMouseEvent;
class QKeyEvent;

class QDBusInterface;

class ChangeUserNickname : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent = nullptr);
    ~ChangeUserNickname();

public:
    void initUI();
    void setConnect();
    void setupStatus();

public:
    QVBoxLayout * mainVerLayout;
    QHBoxLayout * titleHorLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * userNameHorLayout;
    QHBoxLayout * nickNameHorLayout;
    QHBoxLayout * tipHorLayout;
    QVBoxLayout * nickNameWithTipVerLayout;
    QHBoxLayout * computerNameHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;

    QPushButton * closeBtn;
    QPushButton * cancelBtn;
    QPushButton * confirmBtn;

    QLabel * userNameLabel;
    QLabel * nickNameLabel;
    QLabel * computerNameLabel;
    QLabel * tipLabel;

    QLineEdit * userNameLineEdit;
    QLineEdit * nickNameLineEdit;
    QLineEdit * computerNameLineEdit;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *);

private:
    bool setTextDynamicInNick(QLabel * label, QString string);

private:
    QString realname;
    QString oldName;
    QStringList namesIsExists;

    QDBusInterface * cniface;

};

#endif // CHANGEUSERNICKNAME_H
