#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <QPushButton>

class IconButton : public QPushButton
{
    Q_OBJECT
public:
    IconButton(QString iconFileName, QIcon icon, QWidget *parent = nullptr);
    ~IconButton();
    QPixmap loadSvg(const QString &path, int size = 16);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    void reLoadIcon();
private:
    QString iconName;
    QIcon icon;
};


#endif // ICONBUTTON_H
