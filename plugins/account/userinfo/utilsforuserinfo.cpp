#include "utilsforuserinfo.h"

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QDebug>

#include "elipsemaskwidget.h"

UtilsForUserinfo::UtilsForUserinfo(QObject *parent) : QObject(parent)
{

    mainItemFrame = new QFrame();

    logoBtn = new QPushButton();
    logoBtn->setFixedSize(QSize(50, 50));
    logoBtn->setIconSize(QSize(48, 48));

    nickNameLabel = new QLabel();
    nickNameLabel->setFixedHeight(25);

    typeLabel = new QLabel();
    typeLabel->setFixedHeight(20);


    changePwdBtn = new QPushButton();
    changePwdBtn->setFlat(true);
    changePwdBtn->setText(tr("Passwd"));
    changeTypeBtn = new QPushButton();
    changeTypeBtn->setFlat(true);
    changeTypeBtn->setText(tr("Type"));
    delUserBtn = new QPushButton();
    delUserBtn->setFlat(true);
    delUserBtn->setText(tr("Del"));

    connect(changePwdBtn, &QPushButton::clicked, this, [=]{
        emit changePwdBtnPressed();
    });
    connect(changeTypeBtn, &QPushButton::clicked, this, [=]{
        emit changeTypeBtnPressed();
    });
    connect(logoBtn, &QPushButton::clicked, this, [=]{
        emit changeLogoBtnPressed();
    });
    connect(delUserBtn, &QPushButton::clicked, this, [=]{
        emit deleteUserBtnPressed();
    });

}

QFrame * UtilsForUserinfo::buildItemForUsers(){

    //圆形头像遮罩
    ElipseMaskWidget * currentElipseMaskWidget = new ElipseMaskWidget(logoBtn);
    currentElipseMaskWidget->setGeometry(0, 0, logoBtn->width(), logoBtn->height());


    subUserInfoVerLayout = new QVBoxLayout();
    subUserInfoVerLayout->setSpacing(0);
    subUserInfoVerLayout->setContentsMargins(0, 0, 0, 0);
//    subUserInfoVerLayout->addStretch();
    subUserInfoVerLayout->addWidget(nickNameLabel, Qt::AlignHCenter);
    subUserInfoVerLayout->addWidget(typeLabel, Qt::AlignHCenter);
//    subUserInfoVerLayout->addStretch();

    mUserInfoHorLayout = new QHBoxLayout();
    mUserInfoHorLayout->setSpacing(8);
    mUserInfoHorLayout->setContentsMargins(16, 0, 16, 0);
    mUserInfoHorLayout->addWidget(logoBtn);
    mUserInfoHorLayout->addLayout(subUserInfoVerLayout);
    mUserInfoHorLayout->addStretch();
    mUserInfoHorLayout->addWidget(changePwdBtn);
    mUserInfoHorLayout->addWidget(createVLine(mainItemFrame, 10));
    mUserInfoHorLayout->addWidget(changeTypeBtn);
    mUserInfoHorLayout->addWidget(createVLine(mainItemFrame, 10));
    mUserInfoHorLayout->addWidget(delUserBtn);

    mainUserVerLayout = new QVBoxLayout();
    mainUserVerLayout->setSpacing(0);
    mainUserVerLayout->setContentsMargins(0, 0, 0, 0);
    mainUserVerLayout->addLayout(mUserInfoHorLayout);
    mainUserVerLayout->addWidget(createHLine(mainItemFrame));

    mainItemFrame->setMinimumSize(QSize(550, 60));
    mainItemFrame->setMaximumSize(QSize(16777215, 60));
    mainItemFrame->setFrameShape(QFrame::NoFrame);
    mainItemFrame->setLayout(mainUserVerLayout);

    return mainItemFrame;
}

void UtilsForUserinfo::refreshUserLogo(QString logo){
    QPixmap iconfile = makeRoundLogo(logo, logoBtn->width(), logoBtn->height(), logoBtn->width()/2);
    logoBtn->setIcon(iconfile);
}

void UtilsForUserinfo::refreshUserNickname(QString name){
    if (setUtilsTextDynamic(nickNameLabel, name)){
        nickNameLabel->setToolTip(name);
    }
}

void UtilsForUserinfo::refreshUserType(int type){
    QString t = _accountTypeIntToString(type);
    if (setUtilsTextDynamic(typeLabel, t)){
        typeLabel->setToolTip(t);
    }
}

void UtilsForUserinfo::refreshDelStatus(bool enabled){
    delUserBtn->setEnabled(enabled);
}

void UtilsForUserinfo::refreshPwdStatus(bool enabled){
    changePwdBtn->setEnabled(enabled);
}

void UtilsForUserinfo::refreshTypeStatus(bool enabled){
    changeTypeBtn->setEnabled(enabled);
}

void UtilsForUserinfo::setObjectPathData(QString op){
    mainItemFrame->setObjectName(op);
}

bool UtilsForUserinfo::setUtilsTextDynamic(QLabel *label, QString string){
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

QFrame * UtilsForUserinfo::createHLine(QFrame *f, int len){
    QFrame *line = new QFrame(f);
    if (len == 0){
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
    } else {
        line->setMinimumSize(QSize(len, 1));
        line->setMaximumSize(QSize(len, 1));
    }

    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QFrame * UtilsForUserinfo::createVLine(QFrame *f, int len){
    QFrame *line = new QFrame(f);
    if (len == 0){
        line->setMinimumSize(QSize(1, 0));
        line->setMaximumSize(QSize(1, 16777215));
    } else {
        line->setMinimumSize(QSize(1, len));
        line->setMaximumSize(QSize(1, len));
    }

    line->setLineWidth(0);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void UtilsForUserinfo::userPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList){
    Q_UNUSED(property);
    Q_UNUSED(propertyList);

    if (propertyMap.keys().contains("IconFile")){
        QString current = propertyMap.value("IconFile").toString();

        refreshUserLogo(current);
    }

    if (propertyMap.keys().contains("AccountType")){
        int current = propertyMap.value("AccountType").toInt();

        refreshUserType(current);
    }

}

QString UtilsForUserinfo::_accountTypeIntToString(int type){
    QString atype;
    if (type == 0)
        atype = tr("Standard");
    else if (type == 1)
        atype = tr("Admin");

    return atype;
}

QPixmap UtilsForUserinfo::makeRoundLogo(QString logo, int wsize, int hsize, int radius)
{
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
