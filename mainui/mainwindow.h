#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QLabel>
#include <QListWidgetItem>

#include <QProcess>

#include <QDir>
#include <QPluginLoader>

#include <QDebug>

#include "interface.h"
#include "mainpagewidget.h"
#include "modulepagewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QStringList systemStringList;

    void initUI();
    void backToMain();

    QMap<QString, QObject *> export_module(int);


private:
    Ui::MainWindow *ui;

    QPoint dragPos;
    bool mousePress;

    QDir moduleDir;
    QDir pluginDir;
    QDir pluginsDir;

    ModulePageWidget * modulepageWidget;
    MainPageWidget * mainpageWidget;

    QList <QMap<QString, QObject *>> modulesList;

    void loadPlugins();

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

public slots:
    void pluginClicked_cb(QObject * plugin);
};

#endif // MAINWINDOW_H
