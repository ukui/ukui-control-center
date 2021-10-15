#include "passwordbar.h"

#include <QPainter>

PasswordBar::PasswordBar(QWidget *parent):
    QWidget(parent)
{
    initUI();
}

PasswordBar::~PasswordBar()
{
    if (m_style != nullptr) {
        delete m_style;
    }
}

void PasswordBar::initUI()
{
    m_bitLen = 6;               // 密码位数
    m_ballRadius = 8;
    m_fillBallCnt = 0;
    m_ballSpan = m_ballRadius << 1;     // 密码间的跨度

    adjSize();

    const QByteArray id_1(UKUI_QT_STYLE);
    if (QGSettings::isSchemaInstalled(id_1)) {
        m_style = new QGSettings(id_1);
    }
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

    QColor ballColor;
    QString themeName = m_style->get(UKUI_STYLE_KEY).toString();
    if( themeName == "ukui-light" || themeName == "ukui-default" | themeName == "ukui" ) {
        ballColor = QColor(0,0,0,255);         // isNightMode=false
    } else {
        ballColor = QColor(255,255,255,255);   // isNightMode=true
    }

    int beginx = 1;
    for(int i = 0;i < m_bitLen;i++)
    {
        if(i){
            beginx += m_ballSpan + m_ballRadius * 2;
        }
        if(i < m_fillBallCnt)
            painter.setBrush(ballColor);
        else
            painter.setBrush(QColor(255,255,255,0));

        painter.drawEllipse(QPoint(beginx + m_ballRadius, m_ballRadius + 1), m_ballRadius, m_ballRadius);
        //painter.drawRoundedRect(rect(), m_ballRadius, m_ballRadius);
    }
}
