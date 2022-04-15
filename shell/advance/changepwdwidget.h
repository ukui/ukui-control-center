#ifndef CHANGEPWDWIDGET_H
#define CHANGEPWDWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDBusInterface>
#include <../commonComponent/Label/fixlabel.h>

class ChangePwdWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChangePwdWidget(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    void initUi();
    void setConnect();
    void resultNotice();
    void isActive();
    QString getPwd();
    void setPwd(QString pwd);

private:
    QLineEdit *mOriginalEdit = nullptr;
    QLineEdit *mNewPwdEdit = nullptr;
    QLineEdit *mVerifyEdit = nullptr;

    QPushButton *mCancelBtn = nullptr;
    QPushButton *mConfirmBtn = nullptr;

    FixLabel *mHintLabel_1 = nullptr;
    FixLabel *mHintLabel_2 = nullptr;
    FixLabel *mHintLabel_3 = nullptr;

    QDBusInterface *mUkccInterface = nullptr;

private slots:
    void newpwdInputSlot(const QString &pwd);
    void verifypwdInputSlot(const QString &pwd);
    void orignalpwdinoutSlot(const QString &pwd);

public slots:
    void clearContent();

Q_SIGNALS:
    void close();

};

#endif // CHANGEPWDWIDGET_H
