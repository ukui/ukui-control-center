#ifndef NUMBERSBUTTON_H
#define NUMBERSBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QGSettings/QGSettings>
#include "picturetowhite.h"

#define UKUI_QT_STYLE                      "org.ukui.style"
#define UKUI_STYLE_KEY                     "style-name"

class NumbersButton : public QWidget
{
    Q_OBJECT

public:
    NumbersButton(QWidget *parent = nullptr);

Q_SIGNALS:
    void numbersButtonPress(int btn_id);

private:
    void initUI();
    void initConnect();
    void setQSS();

private:
    QPushButton *m_pNumerPressBT[12]; // 0~9 是数字按键 10是删除键 11是清空
    PictureToWhite *m_pPictureToWhite;
    QGSettings *m_style = nullptr;
};

#endif // NUMBERSBUTTON_H
