#include "changefacedialog.h"
#include "ui_changefacedialog.h"

#include <QDebug>

#define FACEPATH "/usr/share/pixmaps/faces/"

ChangeFaceDialog::ChangeFaceDialog(QWidget *parent) :
    ui(new Ui::ChangeFaceDialog)
{
    this->setParent(parent);
    ui->setupUi(this);
    show_faces();
}

ChangeFaceDialog::~ChangeFaceDialog()
{
    delete ui;
}

void ChangeFaceDialog::show_faces(){

    //遍历头像目录
    QStringList facesList;
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)){
        facesList.append(FACEPATH + filename);
    }

    //设置listwidget
    QSize IMAGE_SIZE(64, 64);
    QSize ITEM_SIZE(70, 70);
    ui->listWidget->setIconSize(IMAGE_SIZE);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(5);

    for (int row = 0; row < facesList.count(); row++){
        QPixmap pixmap(facesList[row]);
        QListWidgetItem * item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)), "");
        item->setSizeHint(ITEM_SIZE);
        item->setData(Qt::UserRole, facesList[row]);
        delitemMap.insert(facesList[row], item);
        ui->listWidget->insertItem(row, item);
    }
}

void ChangeFaceDialog::set_face_label(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));
}

void ChangeFaceDialog::set_username_label(QString username){
    ui->usernameLabel->setText(username);
}

void ChangeFaceDialog::set_account_type_label(QString atype){
    ui->typeLabel->setText(atype);
}

void ChangeFaceDialog::set_face_list_status(QString facefile){
    QMap<QString, QListWidgetItem *>::iterator it = delitemMap.find(facefile);

    if (it != delitemMap.end()){
        QListWidgetItem * item = (QListWidgetItem *) it.value();
        ui->listWidget->setCurrentItem(item);
    }
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(item_changed_slot(QListWidgetItem *, QListWidgetItem * )));
}

void ChangeFaceDialog::item_changed_slot(QListWidgetItem * current, QListWidgetItem *previous){
    QString facefile = current->data(Qt::UserRole).toString();

    //show dialog更新头像
    set_face_label(facefile);

    emit face_file_send(facefile, ui->usernameLabel->text());
}
