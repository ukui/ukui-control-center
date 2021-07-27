#include "clock.h"
#include <QTime>
#include <QDebug>

Clock::Clock(QWidget *parent)
    : QWidget(parent)
{
    // 构造定时器，设置超时为 1 秒
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

Clock::~Clock()
{

}
QRectF Clock::textRectF(double radius, int pointSize, double angle)
{
    QRectF rectF;
    rectF.setX(radius*qCos(angle*M_PI/180.0) - pointSize*2);
    rectF.setY(radius*qSin(angle*M_PI/180.0) - pointSize/2.0);
    rectF.setWidth(pointSize * 4);
    rectF.setHeight(pointSize + 2);
    return rectF;
}

void Clock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 时针、分针、秒针位置 - 多边形
    static const QPoint hourHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -65)
    };
    static const QPoint minuteHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -65)
    };

    static const QPoint secondHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -80)
    };

    // 时针、分针、秒针颜色
    QColor hourColor(0, 0, 0, 200);
    QColor minuteColor(105,105,105, 150);
    QColor secondColor(255,0,0, 150);

    int side = qMin(width(), height());
    QTime time = QTime::currentTime();

    QPainter painter(this);









    painter.setRenderHint(QPainter::Antialiasing);
    // 平移坐标系原点至中心点
    painter.translate(width() / 2, height() / 2);
    // 缩放
    painter.scale(side / 200.0, side / 200.0);

    //绘制圆

//    QBrush brush;//定义画刷
//    brush.setColor(QColor(245,182,96));//设置画刷颜色
//    brush.setStyle(Qt::SolidPattern);//设置样式


    QPen pen;//定义画笔
    pen.setWidth(2);//设置画笔宽度
    pen.setColor(QColor(192,192,192));//rgb设置颜色
    pen.setStyle(Qt::SolidLine);//设置风格
    painter.setPen(pen);//将画笔交给画家
    painter.drawEllipse(QPoint(0,0),98,98);//画圆

    // 绘制时针
    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    painter.save();
    // 每圈360° = 12h 即：旋转角度 = 小时数 * 30°
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));


    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    painter.setPen(hourColor);

    // 绘制小时线 （360度 / 12 = 30度）
//    for (int i = 0; i < 12; ++i) {
//        painter.drawLine(88, 0, 96, 0);
//        painter.rotate(30.0);
//    }



    painter.setPen(Qt::black);
    int radius = 100;
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    int pointSize = font.pointSize();
   // qDebug()<<"pointSize  = ="<<font.pointSize();

    // 绘制小时文本
    int nHour = 0;
    for (int i = 0; i < 12; ++i) {
        nHour = i + 3;
        if (nHour > 12)
            nHour -= 12;
        painter.drawText(textRectF(radius*0.8, pointSize, i * 30), Qt::AlignCenter, QString::number(nHour));
    }

    // 绘制分针
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);

    painter.save();
    // 每圈360° = 60m 即：旋转角度 = 分钟数 * 6°
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(minuteHand, 3);
    painter.restore();

    painter.setPen(minuteColor);

    // 绘制分钟线 （360度 / 60 = 6度）
//    for (int j = 0; j < 60; ++j) {
//        if ((j % 5) != 0)
//            painter.drawLine(92, 0, 96, 0);
//        painter.rotate(6.0);
//    }

    // 绘制秒针
    painter.setPen(Qt::NoPen);
    painter.setBrush(secondColor);

    painter.save();
    // 每圈360° = 60s 即：旋转角度 = 秒数 * 6°
    painter.rotate(6.0 * time.second());
    painter.drawConvexPolygon(secondHand, 3);
    painter.restore();
}

