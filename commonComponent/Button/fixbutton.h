#ifndef _FIXBUTTON_H_
#define _FIXBUTTON_H_
#include <QPushButton>

class FixButton : public QPushButton
{
    Q_OBJECT
public:
    FixButton(QWidget *parent = nullptr);
    ~FixButton();
    void setText(const QString &text, bool saveTextFlag = true);
private:
    void paintEvent(QPaintEvent *event);

private:
    QString mStr;
};



#endif
