#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QWidget>

class QLabel;

class ThemeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ThemeWidget(QSize iSize, QString name, QStringList iStringList, QWidget *parent = 0);
    ~ThemeWidget();

public:
    void setSelectedStatus(bool status);

public:
    QLabel * selectedLabel;
    QLabel * placeHolderLabel;


};

#endif // THEMEWIDGET_H
