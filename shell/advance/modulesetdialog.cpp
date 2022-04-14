#include "modulesetdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QEvent>
#include <QKeyEvent>

ModuleSetDialog::ModuleSetDialog(QStringList List, QWidget *parent) :
    modulesList(List),
    QDialog(parent)
{
    initUi();
}

void ModuleSetDialog::initUi()
{
    setWindowTitle(tr("Set"));
    this->setFixedSize(720, 480);
    mStackedwidget = new QStackedWidget(this);

    QHBoxLayout*setLyt = new QHBoxLayout(this);
    setLyt->setContentsMargins(8, 8, 8, 8);
    mLeftwidget = initLeftWidget();
    mPluginwidget = new PluginWidget(modulesList, this);
    mPwdsetwidget = new ChangePwdWidget(this);

    connect(mPluginwidget, &PluginWidget::close, [=](){
        this->close();
    });

    connect(mPwdsetwidget, &ChangePwdWidget::close, [=](){
        this->close();
    });

    mStackedwidget->addWidget(mPluginwidget);
    mStackedwidget->addWidget(mPwdsetwidget);

    setLyt->addWidget(mLeftwidget);
    setLyt->addSpacing(32);
    setLyt->addWidget(mStackedwidget);
//    setLyt->addStretch();

}

QWidget *ModuleSetDialog::initLeftWidget()
{
    QWidget *widget = new QWidget;
    widget->setFixedWidth(180);
    leftBtnGroup = new QButtonGroup();

    QVBoxLayout *Lyt = new QVBoxLayout(widget);
    Lyt->setContentsMargins(0, 0, 0, 0);

    mModuleBtn = new QPushButton(widget);
    mModuleBtn->setFixedHeight(36);
    mModuleBtn->setText(tr("Module settings"));

    mModuleBtn->setStyleSheet("QPushButton:hover{background-color: rgba(55,144,250,0.30);border-radius: 4px;}"
                         "QPushButton:checked{background-color: palette(highlight);border-radius: 4px;}"
                         "QPushButton:!checked{border: none;}"
                         "QPushButton{text-align: left}"
                         "QPushButton{padding-left: 16px}");
    mModuleBtn->setCheckable(true);
    mModuleBtn->setChecked(true);
    leftBtnGroup->addButton(mModuleBtn, 0);

    mPwdBtn = new QPushButton(widget);
    mPwdBtn->setFixedHeight(36);
    mPwdBtn->setText(tr("Module pwd"));
    mPwdBtn->setStyleSheet("QPushButton:hover{background-color: rgba(55,144,250,0.30);border-radius: 4px;}"
                           "QPushButton:checked{background-color: palette(highlight);border-radius: 4px;}"
                           "QPushButton:!checked{border: none;}"
                           "QPushButton{text-align: left}"
                           "QPushButton{padding-left: 16px}");
    mPwdBtn->setCheckable(true);
    leftBtnGroup->addButton(mPwdBtn, 1);

    // 信号重载，强制转换
    connect(leftBtnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [=](const int &id) {
        if (id == 0) {
            mModuleBtn->setChecked(true);
            mPwdBtn->setChecked(false);
            mStackedwidget->setCurrentIndex(0);
            mPwdsetwidget->clearContent();
        } else {
            mModuleBtn->setChecked(false);
            mPwdBtn->setChecked(true);
            mStackedwidget->setCurrentIndex(1);
        }
    });


    Lyt->addWidget(mModuleBtn);
    Lyt->addWidget(mPwdBtn);
    Lyt->addStretch();

    return widget;

}

void ModuleSetDialog::setConnect()
{

}

