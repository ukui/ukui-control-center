#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QMap>

class MainWindow;
class CommonInterface;
class KeyValueConverter;

class QListWidgetItem;

namespace Ui {
class ModulePageWidget;
}

class ModulePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModulePageWidget(QWidget *parent = 0);
    ~ModulePageWidget();

public:
    void initUI();
    void switchPage(QObject * plugin);
    void refreshPluginWidget(CommonInterface * plu);
    void highlightItem(QString text);

private:
    Ui::ModulePageWidget *ui;

private:
    MainWindow * pmainWindow;

    KeyValueConverter * mkvConverter;

private:
    QMap<QString, CommonInterface*> pluginInstanceMap;
    QMultiMap<QString, QListWidgetItem*> strItemsMap;//存储功能名与二级菜单item的Map,为了实现高亮

public slots:
    void currentLeftitemChanged(QListWidgetItem * cur, QListWidgetItem * pre);

Q_SIGNALS:
    void widgetChanged(QString text);
};

#endif // MODULEPAGEWIDGET_H
