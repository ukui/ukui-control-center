#ifndef BIOMETRICSWIDGET_H
#define BIOMETRICSWIDGET_H

#include <QWidget>

class BiometricsWidget : public QWidget
{
    Q_OBJECT
public:
    BiometricsWidget(QWidget *parent = nullptr);
    ~BiometricsWidget();
};

#endif // BIOMETRICSWIDGET_H
