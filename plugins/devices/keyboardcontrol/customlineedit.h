#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
//#include <QMouseEvent>
#include <QFocusEvent>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit CustomLineEdit(QString shortcut, QWidget *parent = 0);
    ~CustomLineEdit();

//    virtual void mousePressEvent(QMouseEvent * e);
    virtual void focusOutEvent(QFocusEvent * evt);
    virtual void focusInEvent(QFocusEvent * evt);
    virtual void keyReleaseEvent(QKeyEvent * evt);

private:
    QString _oldshortcut;
    QString _wait;
    QString _setalready;

};

#endif // CUSTOMLINEEDIT_H
