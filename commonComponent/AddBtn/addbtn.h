#ifndef ADDBTN_H
#define ADDBTN_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QGSettings>

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

class AddBtn : public QPushButton
{
    Q_OBJECT
public:
    AddBtn(QWidget *parent = nullptr);
    ~AddBtn();

protected:
    virtual void leaveEvent(QEvent * event);
    virtual void enterEvent(QEvent * event);

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
};

#endif // ADDBTN_H
