#ifndef HOSTNAMEDIALOG_H
#define HOSTNAMEDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "shell/utils/utils.h"

class HostNameDialog : public QDialog
{
    Q_OBJECT
public:
    HostNameDialog(QWidget *parent);
    ~HostNameDialog();

    void InitUi();

private:
    void initConnect();
    void setEdit();
    void setupComponent();
    void setHostname(QString hostname);

private:

    QLineEdit *mHostNameEdit;
    QString mfirsthostname;
    QPushButton *mCancelBtn;
    QPushButton *mConfirmBtn;
    QLabel *mTipLabel;

};

#endif // HOSTNAMEDIALOG_H
