#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickLabel(const QString &text, QWidget *parent = 0);
    ~ClickLabel();

protected:
    void mousePressEvent(QMouseEvent * event);

signals:
    void clicked();
};

#endif // CLICKLABEL_H
