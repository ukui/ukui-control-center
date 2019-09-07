#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>

#include "interface.h"

#include "maincomponent/listwidgetitem.h"

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
    void switch_modulepage(QObject * plugin, int page = 0);
    void update_backbtn_text(int index);
    void update_plugin_widget(CommonInterface * plu, int page);

private:
    Ui::ModulePageWidget *ui;

    MainWindow * pmainWindow;

    QMap<QString, CommonInterface*> pluginInstanceMap;

    QLabel * backiconLabel;
    QLabel * backtextLabel;

public slots:
    void itemClicked_cb(QListWidgetItem * item);

    void backBtnClicked_cb();
    void update_backbtn_text_slot(int index);
    void toggle_plugin_slot(QString pluginname, int plugintype, int page);
};

#endif // MODULEPAGEWIDGET_H
