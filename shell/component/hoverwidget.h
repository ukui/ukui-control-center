#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QWidget>
#include <QEvent>

class HoverWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HoverWidget(QString mname, QWidget *parent = 0);
    ~HoverWidget();

public:
    QString _name;

protected:
    virtual void enterEvent(QEvent * event);
    virtual void leaveEvent(QEvent * event);
    virtual void paintEvent(QPaintEvent * event);

Q_SIGNALS:
    void enterWidget(QString name);
    void leaveWidget(QString name);
};

#endif // HOVERWIDGET_H
