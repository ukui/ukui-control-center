#ifndef _FIXBTN_H_
#define _FIXBTN_H_
#include <QPushButton>


class  FixBtn : public QPushButton
{
    Q_OBJECT
public:
    FixBtn(QWidget *parent = nullptr);
    ~FixBtn();
    void setText(const QString &text, bool saveTextFlag = true);
private:
    void paintEvent(QPaintEvent *event);

private:
    QString mStr;
};



#endif
