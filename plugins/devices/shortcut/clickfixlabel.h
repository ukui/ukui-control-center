#ifndef CLICKFIXLABEL_H
#define CLICKFIXLABEL_H
#include <QLabel>
class ClickFixLabel : public QLabel
{
    Q_OBJECT
public:
    ClickFixLabel(QWidget *parent = nullptr);
    ~ClickFixLabel();
    void setText(const QString & text, bool saveTextFlag = true);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
Q_SIGNALS:
    void doubleClicked();
private:
    QString mStr;
};



#endif // CLICKFIXLABEL_H
