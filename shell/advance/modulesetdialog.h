#ifndef MODULESETDIALOG_H
#define MODULESETDIALOG_H

#include <QObject>
#include <QDialog>
#include <QStackedWidget>
#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>

#include "pluginwidget.h"
#include "changepwdwidget.h"

class ModuleSetDialog : public QDialog
{
    Q_OBJECT
public:
    ModuleSetDialog(QStringList List, QWidget *parent = nullptr);

private:
    void initUi();
    QWidget *initLeftWidget();
    void setConnect();

private:
    QStringList modulesList;
    QStackedWidget *mStackedwidget = nullptr;
    QWidget *mLeftwidget = nullptr;
    PluginWidget *mPluginwidget = nullptr;
    ChangePwdWidget *mPwdsetwidget = nullptr;

    QButtonGroup *leftBtnGroup;
    QPushButton *mModuleBtn;
    QPushButton *mPwdBtn;
};

#endif // MODULESETDIALOG_H
