#ifndef PRIVACYDIALOG_H
#define PRIVACYDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QTextBrowser>
#include <ukcc/widgets/titlelabel.h>

class PrivacyDialog  : public QDialog
{
    Q_OBJECT
public:
    PrivacyDialog(QWidget *parent);
    ~PrivacyDialog();
private:
    void initUi();

};

#endif // PRIVACYDIALOG_H
