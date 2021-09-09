#ifndef DROWNLABEL_H
#define DROWNLABEL_H

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class DrownLabel : public QLabel
{
    Q_OBJECT
public:
    explicit DrownLabel(QWidget * parent = nullptr);
    ~DrownLabel();
    void setDropDownStatus(bool status);
private:
    bool isChecked = false;
private:
    void loadPixmap(bool isChecked);
protected:
    virtual void mousePressEvent(QMouseEvent * event);

Q_SIGNALS:
    void labelClicked();
};

#endif // DROWNLABEL_H
