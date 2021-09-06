#ifndef PASSWORDBAR_H
#define PASSWORDBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class PasswordBar : public QWidget
{
public:
    PasswordBar(QWidget *parent = nullptr);
    void setLength(int l);
    void setBallRadius(int l);
    void setBallSpan(int l);
    void addFillBall();
    void delFillBall();
    void setFillBall(int l);
    int getFillBall();

private:
    void initUI();
    void adjSize();

protected:
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;

private:
    int m_bitLen;
    int m_ballRadius;
    int m_fillBallCnt;
    int m_ballSpan;
};
#endif // PASSWORDBAR_H
