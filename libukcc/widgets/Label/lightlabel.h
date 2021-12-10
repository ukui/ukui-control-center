#ifndef LIGHTLABEL_H
#define LIGHTLABEL_H
#include <QLabel>
#include "libukcc_global.h"

class LIBUKCC_EXPORT LightLabel : public QLabel
{
    Q_OBJECT
public:
    explicit LightLabel(QWidget *parent = nullptr);
    explicit LightLabel(QString text , QWidget *parent = nullptr);
    ~LightLabel();

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // LIGHTLABEL_H
