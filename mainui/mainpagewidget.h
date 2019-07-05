#ifndef MAINPAGEWIDGET_H
#define MAINPAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSignalMapper>

#include "interface.h"

class MainWindow;

namespace Ui {
class MainPageWidget;
}

class MainPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainPageWidget(QWidget *parent = 0);
    ~MainPageWidget();

    void initUI();

private:
    Ui::MainPageWidget *ui;

    MainWindow * pmainWindow;

    QList<QLabel *> delLabelList;
    QList<QFrame *> delFrameList;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

};

#endif // MAINPAGEWIDGET_H
