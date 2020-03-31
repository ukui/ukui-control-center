#ifndef APPDETAIL_H
#define APPDETAIL_H

#include <QDialog>
#include <QGSettings/QGSettings>

#include "SwitchButton/switchbutton.h"

namespace Ui {
class AppDetail;
}

class AppDetail : public QDialog
{
    Q_OBJECT

public:
    explicit AppDetail(QString Name, QString key, QWidget *parent = nullptr);
    ~AppDetail();

private:
    Ui::AppDetail *ui;
    QString appName;
    QString appKey;
    SwitchButton * enablebtn;
    QGSettings * m_gsettings;

private:
    void initUiStatus();
    void initComponent();
    void initConnect();
    void initGSettings();

private slots:
    void confirmbtnSlot();
};

#endif // APPDETAIL_H
