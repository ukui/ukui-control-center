#ifndef PWDDIAOG_H
#define PWDDIAOG_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QPushButton>
#include <QDBusInterface>
#include <../commonComponent/Label/fixlabel.h>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

class PwdDiaog : public QDialog
{
    Q_OBJECT
public:
    PwdDiaog(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    void initUi();
    void setConnect();
    QString getPwd();

private:
    QPushButton *mCancelBtn;
    QPushButton *mUnLockBtn;

    QLabel *mHintLabel;

    QLineEdit *mpwd;

    QDBusInterface *mUkccInterface = nullptr;

private slots:
    void pwdInputSlot(const QString &pwd);

Q_SIGNALS:
    void open();

};

#endif // PWDDIAOG_H
