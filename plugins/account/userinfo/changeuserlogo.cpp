#include "changeuserlogo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QDBusInterface>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemWatcher>

#include <QDebug>
#include "../../../shell/utils/utils.h"
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#include <ukcc/widgets/flowlayout.h>


#define FACEPATH "/usr/share/ukui/faces/"

ChangeUserLogo::ChangeUserLogo(QString n, QString op, QWidget *parent) :
    QDialog(parent),
    name(n)
{
    setFixedSize(QSize(400, 430));

    culiface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());

    logosBtnGroup = new QButtonGroup;

    selected = QString();

    loadSystemLogo();
    initUI();
    setupConnect();
}

ChangeUserLogo::~ChangeUserLogo()
{
    delete culiface;
}

void ChangeUserLogo::loadSystemLogo(){

    logosFlowLayout = new FlowLayout(10, -1, -1);
    logosFlowLayout->setContentsMargins(0, 0, 0, 10);

    // 遍历头像目录
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)) {
        QString fullface = QString("%1%2").arg(FACEPATH).arg(filename);
        // 社区版不加载商业默认头像
        if (Utils::isCommunity() && fullface.endsWith("commercial.png")) {
            continue;
        }
        // 商业版不加载社区默认头像
        if (!Utils::isCommunity() &&fullface.endsWith("community.png")) {
            continue;
        }
        // 升级后 default.png 不存在导致开始菜单无头像显示 #108837
        if (fullface.endsWith("default.png")) {
            continue;
        }

        QPushButton *button = new QPushButton;
        button->setCheckable(true);
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(QSize(64, 64));

        logosBtnGroup->addButton(button);

        button->setIcon(QIcon(fullface));
        button->setIconSize(QSize(59,59));
        ElipseMaskWidget * currentElipseMaskWidget = new ElipseMaskWidget(button);
        currentElipseMaskWidget->setGeometry(0, 0, button->width(), button->height());

        connect(button, &QPushButton::clicked, [=]{
            // show dialog更新头像
            refreshUserLogo(fullface);

            selected = fullface;
        });

        logosFlowLayout->addWidget(button);
    }

    logosFrame = new QFrame;
    logosFrame->setMinimumSize(QSize(352, 142));
    logosFrame->setMaximumSize(QSize(16777215, 142));
    logosFrame->setFrameShape(QFrame::Box);
    logosFrame->setFrameShadow(QFrame::Plain);
    logosFrame->setLayout(logosFlowLayout);
}


void ChangeUserLogo::initUI()
{
    setWindowTitle(tr("User logo"));

    culLogoLabel = new QLabel;
    culLogoLabel->setFixedSize(QSize(48, 48));
    culNickNameLabel = new QLabel;
    culNickNameLabel->setFixedHeight(24);
    culTypeLabel = new QLabel;
    culTypeLabel->setFixedHeight(24);

    culUserInfoVerLayout = new QVBoxLayout;
    culUserInfoVerLayout->setSpacing(4);
    culUserInfoVerLayout->setContentsMargins(0, 0, 0, 0);
//    culUserInfoVerLayout->addStretch();
    culUserInfoVerLayout->addWidget(culNickNameLabel);
    culUserInfoVerLayout->addWidget(culTypeLabel);
//    culUserInfoVerLayout->addStretch();

    culUserHorLayout = new QHBoxLayout;
    culUserHorLayout->setSpacing(8);
    culUserHorLayout->setContentsMargins(0, 0, 0, 0);
    culUserHorLayout->addWidget(culLogoLabel);
    culUserHorLayout->addLayout(culUserInfoVerLayout);

    culNoteLabel = new QLabel;
    culNoteLabel->setFixedHeight(24);
    culNoteLabel->setText(tr("System Logos"));

    culLogoNoteHorLayout = new QHBoxLayout;
    culLogoNoteHorLayout->setSpacing(0);
    culLogoNoteHorLayout->setContentsMargins(0, 0, 0, 0);
    culLogoNoteHorLayout->addWidget(culNoteLabel);


    culMoreLogoBtn = new QPushButton;
    culMoreLogoBtn->setText(tr("Select Local Logo"));

    culMoreLogoHorLayout = new QHBoxLayout;
    culMoreLogoHorLayout->setSpacing(0);
    culMoreLogoHorLayout->setContentsMargins(0, 0, 0, 0);
    culMoreLogoHorLayout->addWidget(culMoreLogoBtn);
    culMoreLogoHorLayout->addStretch();

    culCancelBtn = new QPushButton;
    culCancelBtn->setText(tr("Cancel"));
    culConfirmBtn = new QPushButton;
    culConfirmBtn->setText(tr("Confirm"));

    culBottomBtnsHorLayout = new QHBoxLayout;
    culBottomBtnsHorLayout->setSpacing(16);
    culBottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    culBottomBtnsHorLayout->addStretch();
    culBottomBtnsHorLayout->addWidget(culCancelBtn);
    culBottomBtnsHorLayout->addWidget(culConfirmBtn);

    culMainVerLayout = new QVBoxLayout;
    culMainVerLayout->setContentsMargins(24, 0, 24, 0);
    culMainVerLayout->addSpacing(26);
    culMainVerLayout->addLayout(culUserHorLayout);
    culMainVerLayout->addSpacing(26);
    culMainVerLayout->addLayout(culLogoNoteHorLayout);
    culMainVerLayout->addWidget(logosFrame);
    culMainVerLayout->addSpacing(24);
    culMainVerLayout->addLayout(culMoreLogoHorLayout);
    culMainVerLayout->addSpacing(25);
    culMainVerLayout->addLayout(culBottomBtnsHorLayout);
    culMainVerLayout->addSpacing(24);

    setLayout(culMainVerLayout);
}

QPixmap ChangeUserLogo::makeRoundLogo(QString logo, int wsize, int hsize, int radius){
    QPixmap rectPixmap;
    QPixmap iconcop = QPixmap(logo);

    if (iconcop.width() > iconcop.height()) {
        QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(hsize);
    } else {
        QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(wsize);
    }

    if (rectPixmap.isNull()) {
        return QPixmap();
    }
    QPixmap pixmapa(rectPixmap);
    QPixmap pixmap(radius*2,radius*2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius*2, radius*2, pixmapa);
    return pixmap;
}

void ChangeUserLogo::refreshUserLogo(QString logo){
    culLogoLabel->setPixmap(makeRoundLogo(logo, culLogoLabel->width(), culLogoLabel->height(), culLogoLabel->width()/2));
}

void ChangeUserLogo::requireUserInfo(QString logo, QString type){
    //设置头像
    refreshUserLogo(logo);

    culNickNameLabel->setText(name);
    if (setCulTextDynamic(culNickNameLabel, name)){
        culNickNameLabel->setToolTip(name);
    }

    culTypeLabel->setText(type);
}

void ChangeUserLogo::setupConnect(){

    connect(culCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(culConfirmBtn, &QPushButton::clicked, this, [=]{
        if (selected != "") {
            qDebug() << "selected:" << selected << ";" << __LINE__;
            culiface->call("SetIconFile", selected);

            emit face_file_send(selected);
        }
        close();
    });

    connect(culMoreLogoBtn, &QPushButton::clicked, this, [=]{
        showLocalFaceDialog();
    });
}

void ChangeUserLogo::showLocalFaceDialog(){

    QString filters = "Face files(*.jpg *.jpeg *.png *.svg *.gif)";
    QFileDialog fd(this);
    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom face file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QFile pic(selectedfile);
    int size = pic.size();

    qDebug() << "size is"  << size;
    if (size >= 1048576) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("The avatar is larger than 1M, please choose again"));
        return;
    }

    refreshUserLogo(selectedfile);

    selected = selectedfile;
}

bool ChangeUserLogo::setCulTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > 80) {
        label->setFixedWidth(80);
        str = fontMetrics.elidedText(string, Qt::ElideRight, 80);
        isOverLength = true;
    } else {
        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}
