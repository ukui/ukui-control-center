#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <QPushButton>

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    IconButton(QString iconFileName, QWidget *parent = nullptr);
    ~IconButton();
    QPixmap loadSvg(const QString &path, int size = 16);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
protected:
    void paintEvent(QPaintEvent *e);
private:
    QString iconName;
};


#endif // ICONBUTTON_H
