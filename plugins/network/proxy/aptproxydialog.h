#ifndef APTPROXYDIALOG_H
#define APTPROXYDIALOG_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QGSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include "proxy.h"
#include <ukcc/widgets/fixlabel.h>

class AptProxyDialog : public QDialog
{
    Q_OBJECT
public:
    AptProxyDialog(QGSettings *Keygsettiings ,QWidget *parent = nullptr);
    ~AptProxyDialog();

    void initUi();
private:
    QLineEdit *mHostEdit;
    QLineEdit *mPortEdit;

    QPushButton *mCancelBtn;
    QPushButton *mConfirmBtn;

    QGSettings *mgsettings;

private:
    void initConnect();
     void setupComponent();
};

#endif // APTPROXYDIALOG_H