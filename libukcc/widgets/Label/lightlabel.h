#ifndef LIGHTLABEL_H
#define LIGHTLABEL_H
#include <QLabel>
#include "libukcc_global.h"
#include <ukcc/widgets/fixlabel.h>

class LIBUKCC_EXPORT LightLabel : public FixLabel
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
