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

public:
    QSettings *gesturesetting=nullptr;
    int status = -1;

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent*);

public slots:
    void startMovieSlot();
    void stopMovieSlot();

private:
    Ui::Itemwidget *ui;
    QMovie *movie;
    QPixmap currentpix;

    QGSettings * stylegsetting = nullptr;
    QColor color1;
    QColor color2;
    QColor color1_pre;
    QColor color2_pre;

Q_SIGNALS:
    void picClicked();
};

#endif // ITEMWIDGET_H
