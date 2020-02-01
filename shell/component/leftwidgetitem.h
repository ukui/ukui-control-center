#ifndef LEFTWIDGETITEM_H
#define LEFTWIDGETITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class LeftWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit LeftWidgetItem(QWidget *parent = 0);
    ~LeftWidgetItem();

public:
    void setLabelPixmap(QString filename);
    void setLabelText(QString text);

    void setSelected(bool selected);

    QString text();

private:
    QLabel * iconLabel;
    QLabel * textLabel;

    QWidget * widget;

};

#endif // LEFTWIDGETITEM_H
