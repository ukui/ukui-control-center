#include "deluserdialog.h"
#include "ui_deluserdialog.h"

#include <QDebug>

DelUserDialog::DelUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelUserDialog)
{
    ui->setupUi(this);
    connect(ui->cancelPushBtn, SIGNAL(clicked()), this, SLOT(reject()));

    QSignalMapper * differSignalMapper = new QSignalMapper();
    for (QAbstractButton * button : ui->buttonGroup->buttons()){
        connect(button, SIGNAL(clicked()), differSignalMapper, SLOT(map()));
        differSignalMapper->setMapping(button, button->text());
    }
    connect(differSignalMapper, SIGNAL(mapped(QString)), this, SLOT(btn_clicked_slot(QString)));
}

DelUserDialog::~DelUserDialog()
{
    delete ui;
}

void DelUserDialog::set_face_label(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(64, 64)));
}

void DelUserDialog::set_username_label(QString username){
    ui->usernameLabel->setText(username);
}

void DelUserDialog::btn_clicked_slot(QString key){
    this->accept();
    bool removefile;
    if (ui->removePushBtn->text() == key)
        removefile = true;
    else
        removefile = false;
    emit removefile_send(removefile, ui->usernameLabel->text());
}
