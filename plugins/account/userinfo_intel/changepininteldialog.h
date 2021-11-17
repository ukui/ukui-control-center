#ifndef CHANGEPININTELDIALOG_H
#define CHANGEPININTELDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QSharedPointer>
#include <QVBoxLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QString>
#include <QGSettings/QGSettings>
#include "digitalauthinteldialog.h"
#include "phoneauthinteldialog.h"
#include "changepwdinteldialog.h"
#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"

class QLabel;
class QPushButton;

namespace Ui {
class ChangePinIntelDialog;
}

class ChangePinIntelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePinIntelDialog(QString username, QWidget *parent = nullptr);
    ~ChangePinIntelDialog();
    static int m_isThemeChanged;                                    //主题

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::ChangePinIntelDialog *ui;
    QPushButton *closeBtn;
    DigitalAuthIntelDialog *m_pdigitalAuthWidget;
    PhoneAuthIntelDialog *m_pPhoneAuthWidget;
    ChangePwdIntelDialog *m_pChangePwdDialog;
    QGSettings  *m_style = nullptr;

Q_SIGNALS:
    void EmitThemeChanged(int theme);
    void changepwd();

private slots:
    void setpwdstyle(QString key);
};

#endif // CHANGEPININTELDIALOG_H
