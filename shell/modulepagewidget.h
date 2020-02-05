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

public slots:
    void leftItemClicked(QListWidgetItem * item);

Q_SIGNALS:
    void widgetChanged(QString text);
};

#endif // MODULEPAGEWIDGET_H
