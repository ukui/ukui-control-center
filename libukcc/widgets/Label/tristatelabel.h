#ifndef TRISTATELABEL_H
#define TRISTATELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

class tristateLabel : public QLabel
{
    Q_OBJECT

public:
    tristateLabel(const QString &text, QWidget *parent);
    ~tristateLabel();

    QString abridge(QString text); // 简写英文多长的应用名称
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    bool mMoved = false;
    bool mClicked = false;

signals:
    void clicked();
};

#endif // TRISTATELABEL_H
