#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>

#include "interface.h"
#include "homepagewidget.h"
#include "modulepagewidget.h"

class QPushButton;
class QButtonGroup;
class KeyValueConverter;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QMap<QString, QObject *> exportModule(int);
    void setModuleBtnHightLight(int id);

private:
    Ui::MainWindow *ui;

private:
    HomePageWidget * homepageWidget;
    ModulePageWidget * modulepageWidget;

    QButtonGroup * leftBtnGroup;
    QButtonGroup * leftMicBtnGroup; //

    QDir pluginsDir;
//    QStringList modulesStringList;
    QList<int> moduleIndexList;
    QList<QMap<QString, QObject *>> modulesList;

    KeyValueConverter * kvConverter;

private:
    void setBtnLayout(QPushButton * &pBtn);
    void loadPlugins();
    void initLeftsideBar();
    QPushButton * buildLeftsideBtn(QString bname);

public slots:
    void functionBtnClicked(QObject * plugin);

};

#endif // MAINWINDOW_H
