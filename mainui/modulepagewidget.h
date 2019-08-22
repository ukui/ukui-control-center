#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>

#include "interface.h"

#include "component/listwidgetitem.h"

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
    void update_backbtn_text(int index);

private:
    Ui::ModulePageWidget *ui;

    MainWindow * pmainWindow;

    QMap<QString, QWidget *> widgetMaps;

    QLabel * backiconLabel;
    QLabel * backtextLabel;

public slots:
    void itemClicked_cb(QListWidgetItem * item);

    void backBtnClicked_cb();
    void update_backbtn_text_slot(int index);

};

#endif // MODULEPAGEWIDGET_H
