#ifndef NUMBERSBUTTON_H
#define NUMBERSBUTTON_H

#include <QWidget>
#include <QPushButton>
#include "picturetowhite.h"

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
};

#endif // NUMBERSBUTTON_H
