#include "customlineedit.h"

#include <QDebug>

CustomLineEdit::CustomLineEdit(QString shortcut, QWidget *parent) :
    QLineEdit(parent),
    _oldshortcut(shortcut)
{
    _wait = "New Shortcut...";
    _setalready = "New Shortcut... ";
}

CustomLineEdit::~CustomLineEdit()
{
}

//void CustomLineEdit::mousePressEvent(QMouseEvent *e){
//    if (e->buttons() & Qt::LeftButton){
//        this->setText("New Shortcut...");
//    }
//}

void CustomLineEdit::focusInEvent(QFocusEvent *evt){
    if (this->text() == _oldshortcut)
        this->setText(_wait);
}

void CustomLineEdit::focusOutEvent(QFocusEvent *evt){
    if (this->text() == _wait)
        this->setText(_oldshortcut);
}

void CustomLineEdit::keyReleaseEvent(QKeyEvent *evt){
    if (evt->modifiers() == Qt::MetaModifier)
        qDebug() << "super press";

    if (evt->key() == Qt::Key_Escape){
        this->setText(_oldshortcut);
    }
    else if (int(evt->modifiers()) == 0 && evt->key() != 0 && this->text() == _wait){ //判断当前text，屏蔽掉多余的keyRelease事件触发
        this->setText(_setalready);
        qDebug() << evt->key() << evt->text();
    }
    else if (evt->modifiers() == Qt::ControlModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Ctr + " << evt->key() << evt->text();
    }
    else if (evt->modifiers() == Qt::AltModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Alt + " << evt->key() << evt->text();
    }
    else if (evt->modifiers() == Qt::ShiftModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Shift + " << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Ctr + Alt" << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Ctr + shift" << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        qDebug() << "Alt + shift" << evt->key() << evt->text();
    }
    qDebug() << "press" << evt->key();
}
