#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include "Label/fixlabel.h"

const QString vTen        = "v10";
const QString vTenEnhance = "v10.1";
const QString vFour = "v4";

#define THEME_STYLE_SCHEMA "org.ukui.style"
#define STYLE_NAME_KEY "style-name"
#define CONTAIN_STYLE_NAME_KEY "styleName"
#define UKUI_DEFAULT "ukui-default"
#define UKUI_DARK "ukui-dark"

class StatusDialog : public QDialog
{
    Q_OBJECT
public:
    StatusDialog(QWidget *parent = nullptr);
    ~StatusDialog();

private:
    void initUI();

public:
    QLabel *mLogoLabel;
    QLabel *mVersionLabel_1;
    FixLabel *mVersionLabel_2;
    FixLabel *mStatusLabel_1;
    QLabel *mStatusLabel_2;
    QLabel *mSerialLabel_1;
    FixLabel *mSerialLabel_2;
    FixLabel *mTimeLabel_1;
    QLabel *mTimeLabel_2;
    QPushButton *mExtentBtn;

private:
    QFrame *mVersionFrame;
    QFrame *mStatusFrame;
    QFrame *mSerialFrame;
    QFrame *mTimeFrame;

};

#endif // STATUSDIALOG_H
