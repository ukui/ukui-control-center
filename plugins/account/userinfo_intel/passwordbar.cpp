#include "passwordbar.h"

#include <QPainter>

PasswordBar::PasswordBar(QWidget *parent):
    QWidget(parent)
{
    initUI();
}

void PasswordBar::initUI()
{
    m_bitLen = 6;               // 密码位数
    m_ballRadius = 8;
    m_fillBallCnt = 0;
    m_ballSpan = m_ballRadius << 1;     // 密码间的跨度

    adjSize();
}

void PasswordBar::setLength(int l)
{
    m_bitLen = l;
    adjSize();
}

void PasswordBar::setBallRadius(int l)
{
    m_ballRadius = l;
    adjSize();
}

void PasswordBar::setBallSpan(int l)
{
    m_ballSpan = l;
    adjSize();
}

void PasswordBar::setFillBall(int l)
{
    m_fillBallCnt = l;
    repaint();
}

int PasswordBar::getFillBall()
{
    return m_fillBallCnt;
}

void PasswordBar::adjSize()
{
    int ballD = m_ballRadius << 1;
    setFixedHeight(ballD + 2);
    setFixedWidth(ballD * m_bitLen + 2 + m_ballSpan * (m_bitLen - 1));
    adjustSize();
    repaint();
}

void PasswordBar::addFillBall()
{
    if(m_fillBallCnt == m_bitLen) return;
    ++m_fillBallCnt;
    repaint();
}

void PasswordBar::delFillBall()
{
    if(!m_fillBallCnt) return;
    --m_fillBallCnt;
    repaint();
}

QSize PasswordBar::sizeHint() const{
    return QSize(width(), height());
}

void PasswordBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(QColor(200, 200, 200, 255));
    painter.setRenderHint(QPainter::Antialiasing);

    int beginx = 1;
    for(int i = 0;i < m_bitLen;i++)
    {
        if(i){
            beginx += m_ballSpan + m_ballRadius * 2;
        }
        if(i < m_fillBallCnt)
            painter.setBrush(QColor(200,200,200,255));
        else
            painter.setBrush(QColor(255,255,255,0));

        painter.drawEllipse(QPoint(beginx + m_ballRadius, m_ballRadius + 1), m_ballRadius, m_ballRadius);
        //painter.drawRoundedRect(rect(), m_ballRadius, m_ballRadius);
    }
}
