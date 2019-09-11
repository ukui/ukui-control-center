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
    QList<int> tmpList;
    if (int(evt->modifiers()) == Qt::NoModifier && evt->key() != 0 && this->text() == _wait){ //判断当前text，屏蔽掉多余的keyRelease事件触发
        this->setText(_setalready);
        tmpList.append(evt->key());
//        qDebug() << evt->key() << evt->text();
    }
    else if (evt->modifiers() == Qt::ControlModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Control);
        tmpList.append(evt->key());
//        qDebug() << "Ctr + " << evt->key() << evt->text() << (int)Qt::ControlModifier << (int)Qt::Key_Control;
    }
    else if (evt->modifiers() == Qt::AltModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Alt);
        tmpList.append(evt->key());
//        qDebug() << "Alt + " << evt->key() << evt->text();
    }
    else if (evt->modifiers() == Qt::ShiftModifier && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Shift);
        tmpList.append(evt->key());
//        qDebug() << "Shift + " << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Control);
        tmpList.append(Qt::Key_Alt);
        tmpList.append(evt->key());
//        qDebug() << "Ctr + Alt" << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Control);
        tmpList.append(Qt::Key_Shift);
        tmpList.append(evt->key());
//        qDebug() << "Ctr + shift" << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::AltModifier | Qt::ShiftModifier)) && evt->key() != 0 && this->text() == _wait){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Alt);
        tmpList.append(Qt::Key_Shift);
        tmpList.append(evt->key());
//        qDebug() << "Alt + shift" << evt->key() << evt->text();
    }
    else if ((evt->modifiers() == (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier) && evt->key() != 0 && this->text() == _wait)){
        this->setText(_setalready);
        tmpList.append(Qt::Key_Control);
        tmpList.append(Qt::Key_Alt);
        tmpList.append(Qt::Key_Shift);
        tmpList.append(evt->key());
    }
    if (tmpList.length() > 0)
        emit shortcut_code_signals(tmpList);
}
