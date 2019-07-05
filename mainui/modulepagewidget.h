#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QListWidget>

#include "interface.h"

class MainWindow;

namespace Ui {
class ModulePageWidget;
}

class ModulePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModulePageWidget(QWidget *parent = 0);
    ~ModulePageWidget();

    void initUI();
    void setup_component(QObject * plugin);

private:
    Ui::ModulePageWidget *ui;

    MainWindow * pmainWindow;

    QMap<QString, QWidget *> widgetMaps;

public slots:
    void itemClicked_cb(QListWidgetItem * item);

    void backBtnClicked_cb();

};

#endif // MODULEPAGEWIDGET_H
