#ifndef LISTWIDGETITEM_H
#define LISTWIDGETITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class ListWidgetItem : public QWidget
{
    Q_OBJECT

public:
    explicit ListWidgetItem(QWidget *parent = nullptr);
    ~ListWidgetItem();

    void setLabelPixmap(QString filename);
    void setLabelText(QString text);
    QString text();

private:
    QLabel * iconLabel;
    QLabel * textLabel;
};

#endif // LISTWIDGETITEM_H
