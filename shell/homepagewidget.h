#ifndef HOMEPAGEWIDGET_H
#define HOMEPAGEWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
class HomePageWidget;
}

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(QWidget *parent = 0);
    ~HomePageWidget();

public:
    void initUI();

private:
    Ui::HomePageWidget *ui;

private:
    MainWindow * pmainWindow;
};

#endif // HOMEPAGEWIDGET_H
