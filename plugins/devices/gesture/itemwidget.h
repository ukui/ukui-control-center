#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QMovie>
#include <QString>
#include <QEvent>
#include <QGSettings/QGSettings>
namespace Ui {
class Itemwidget;
}

class Itemwidget : public QWidget
{
    Q_OBJECT

public:
    explicit Itemwidget(QString itemname,QWidget *parent = nullptr);
    ~Itemwidget();
    QSettings *gesturesetting=nullptr;
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
private slots:
    void on_pushButton_clicked();
    void paintEvent(QPaintEvent*);

private:
    Ui::Itemwidget *ui;
    QMovie *movie;
    QPixmap currentpix;
    int status=-1;//播放状态
    QGSettings * stylegsetting=nullptr;
    QColor color1;
    QColor color2;
    QColor color1_pre;
    QColor color2_pre;
//    QPalette pe1;

};

#endif // ITEMWIDGET_H
