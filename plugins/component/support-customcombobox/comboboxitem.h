#ifndef COMBOBOXITEM_H
#define COMBOBOXITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>

class ComboboxItem : public QWidget
{
    Q_OBJECT

public:
    explicit ComboboxItem(QWidget *parent = nullptr);
    ~ComboboxItem();

    void setLabelContent(QString content);
    QString getLabelContent();

protected:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);

private:
    QLabel * imgLabel;
    QLabel * textLabel;
    QHBoxLayout * mainlayout;

    bool mpress;

Q_SIGNALS:
    void chooseItem(QString text);

};

#endif // COMBOBOXITEM_H
