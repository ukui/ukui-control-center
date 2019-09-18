#include "addautoboot.h"
#include "ui_addautoboot.h"

#include <QDebug>

#define DESKTOPPATH "/etc/xdg/autostart/"

AddAutoBoot::AddAutoBoot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAutoBoot)
{
    ui->setupUi(this);

    connect(ui->openBtn, SIGNAL(clicked(bool)), this, SLOT(open_desktop_dir_slots()));
    connect(ui->cancelBtn, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->certainBtn, &QPushButton::clicked, this, [=]{emit autoboot_adding_signals(ui->nameLineEdit->text(), ui->execLineEdit->text(), ui->commentLineEdit->text());close();});
}

AddAutoBoot::~AddAutoBoot()
{
    delete ui;
}

void AddAutoBoot::open_desktop_dir_slots(){
    QString filters = "Desktop files(*.desktop)";
    QFileDialog fd;
    fd.setDirectory(DESKTOPPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect autoboot desktop"));
    fd.setLabelText(QFileDialog::Accept, "Select");

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QByteArray ba;
    ba = selectedfile.toLatin1();

    //解析desktop文件
    GKeyFile * keyfile;
    char *name, * comment, * exec;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)){
        g_key_file_free (keyfile);
        return;
    }

    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

    if (ui->nameLineEdit->text().isEmpty())
        ui->nameLineEdit->setText(QString(name));
    if (ui->execLineEdit->text().isEmpty())
        ui->execLineEdit->setText(QString(exec));
    if (ui->commentLineEdit->text().isEmpty())
        ui->commentLineEdit->setText(QString(comment));
}
