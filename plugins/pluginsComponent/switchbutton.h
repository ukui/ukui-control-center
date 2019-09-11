#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QEvent>

class SwitchButton : public QWidget
{
    Q_OBJECT

public:
    SwitchButton(QWidget *parent = 0);
    ~SwitchButton();

    void setChecked(bool checked);

    bool isChecked();

protected:
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void drawBg(QPainter * painter);
    void drawSlider(QPainter * painter);

private:
    bool checked;

    QColor borderColorOff;

    QColor bgColorOff;
    QColor bgColorOn;

    QColor sliderColorOff;
    QColor sliderColorOn;

    int space; //滑块离背景间隔
    int rectRadius; //圆角角度

    int step; //移动步长
    int startX;
    int endX;

    QTimer * timer;


private slots:
    void updatevalue();


Q_SIGNALS:
    void checkedChanged(bool checked);

};

#endif // SWITCHBUTTON_H
