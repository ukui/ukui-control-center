#ifndef CERTIFICATIONDIALOG_H
#define CERTIFICATIONDIALOG_H

#include <QDialog>

#define HTTP_PROXY_SCHEMA         "org.gnome.system.proxy.http"
#define HTTP_AUTH_KEY         "use-authentication"
#define HTTP_AUTH_USER_KEY        "authentication-user"
#define HTTP_AUTH_PASSWD_KEY      "authentication-password"

#include <QGSettings/QGSettings>

namespace Ui {
class CertificationDialog;
}

class CertificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CertificationDialog(QWidget *parent = 0);
    ~CertificationDialog();

    void component_init();
    void status_init();

private:
    Ui::CertificationDialog *ui;

    QGSettings * cersettings;

private slots:
    void cerCheckBox_clicked_slot(bool status);

    void user_edit_changed_slot(QString edit);
    void pwd_edit_changed_slot(QString edit);
};

#endif // CERTIFICATIONDIALOG_H
