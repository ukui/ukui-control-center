#include "deletepkglistwig.h"

#define ORG_UKUI_STYLE "org.ukui.style"
#define GSETTING_KEY "systemFontSize"

const QString FIND_DES_LABLE_TYPE = "FIND_DES_LABLE_TYPE";//历史更新模块标签
const QString FIND_REASON_LABLE_TYPE = "FIND_REASON_LABLE_TYPE";//历史更新模块标签
const QString IS_SELECT = "IS_SELECT";//历史更新模块标签
const int WIDTH = 240 - 2;
const int LINE_SPACING = 2;
const int TOP_MARGIN = 5;//上(下)边距
const int RIGHT_MARGIN = 3;//右边距
const int LEFT_MARGIN = 9;//左边距文字和lable本身有边距

DeletePkgListWig::DeletePkgListWig(QLabel* destab)
{
    this->mDesTab = destab;

    initUI();
    gsettingInit();
}
DeletePkgListWig::~DeletePkgListWig()
{
    debName->deleteLater();
    hl1->deleteLater();
    vl1->deleteLater();
}
void DeletePkgListWig::initUI()
{
    font.setBold(true);

    /* 窗口初始化 */
    this->setFrameStyle(QFrame::Box);

    /* 实例化控件 */
    debName = new QLabel;
    debName->setWordWrap(true);
    debName->setFixedWidth(WIDTH - LEFT_MARGIN - LEFT_MARGIN);
    debName->setFont(font);

    /* 布局 */
    hl1 = new QHBoxLayout;
    vl1 = new QVBoxLayout;

    hl1->setSpacing(0);
    hl1->setMargin(0);
    vl1->setSpacing(0);
    vl1->setMargin(0);

    hl1->addSpacing(LEFT_MARGIN);
    hl1->addWidget(debName);
    hl1->addSpacing(RIGHT_MARGIN);

    vl1->addSpacing(TOP_MARGIN);
    vl1->addLayout(hl1);
    vl1->addSpacing(TOP_MARGIN);

    this->setLayout(vl1);
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void DeletePkgListWig::setAttribute(QString &mname, QString &mdescription,QString &deletereason)
{
    //debName->setText(mname);

    this->mNameLabel = mname;

    /* 单行显示 ， 超出范围加... 和悬浮框 */
    QFontMetrics nameFontMetrics(debName->font());
    int nameFontSize = nameFontMetrics.width(mname);
    int nameLableWidth = debName->width();
    QString nameFormatBody = mname;
    if (nameFontSize > (nameLableWidth - 10)) {
        nameFormatBody = nameFontMetrics.elidedText(nameFormatBody , Qt::ElideRight , nameLableWidth - 10);
        debName->setText(nameFormatBody);
        this->nameTipStatus = true;
        debName->setToolTip(mname);
    } else {
        this->nameTipStatus = false;
        debName->setText(nameFormatBody);
    }

    debDescription=mdescription;
    debdeletereason=deletereason;
}

QSize DeletePkgListWig::getTrueSize()
{
    QSize lsize =this->layout()->sizeHint();
    //lsize.setHeight(lsize.rheight()+LINE_SPACING*2);
    return lsize;
}

//鼠标 点击
void DeletePkgListWig::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::LeftButton)
    {
        setDescription();
        selectStyle();
    }
    if(e->button() == Qt::RightButton)
    {
        clearStyleSheet();
    }

}

void DeletePkgListWig::selectStyle()
{
    //如果上次选中的也是自身
    if(this->statusTip()==IS_SELECT)
        return;
    //清除其他item选中样式及标签
    QList<DeletePkgListWig *> list = this->parent()->findChildren<DeletePkgListWig *>();
    for(DeletePkgListWig *tmp : list)
    {
        if(tmp->statusTip()==IS_SELECT)
        {
            tmp->clearStyleSheet();
            /* 重设tips */
            if (tmp->nameTipStatus == true) {
                tmp->debName->setToolTip(tmp->mNameLabel);
            } else {
                tmp->debName->setToolTip(QString(""));
            }
        }
    }

    //设置选中样式及标签
    debName->setToolTip(QString(""));
    debName->setStyleSheet("color:#fff;");
    this->setStyleSheet("QFrame{background-color:rgba(55, 144, 250, 1);border-radius:4px}");
    this->setStatusTip(IS_SELECT);

    //详细内容
    setDescription();
}

void DeletePkgListWig::clearStyleSheet()
{
    debName->setStyleSheet("");
    this->setStyleSheet("");
    this->setStatusTip("");
}

void DeletePkgListWig::setDescription()
{
    if (this->mDesTab != nullptr) {
        this->mDesTab->setFont(this->font);
        this->mDesTab->setText(this->mNameLabel);
    }

    QObject *findwig = this->parent();
    while(findwig != nullptr)
    {
        if(findwig->objectName()==FIND_DES_LABLE_TYPE)
        {
            break;
        }
        findwig = findwig->parent();
    }
    QTextEdit *dsc = findwig->findChild<QTextEdit *>(FIND_DES_LABLE_TYPE,Qt::FindChildrenRecursively);
    if(dsc==nullptr)
        qDebug()<<"找不到要赋值的窗口";
    else
        dsc->setText(debDescription);
    QLabel *deletereasonlab = findwig->findChild<QLabel *>(FIND_REASON_LABLE_TYPE,Qt::FindChildrenRecursively);
    if(deletereasonlab==nullptr)
        qDebug()<<"找不到要赋值的窗口";
    else
        deletereasonlab->setText(debdeletereason);
}

void DeletePkgListWig::gsettingInit()
{
    const QByteArray style_id(ORG_UKUI_STYLE);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key){
        if (key==GSETTING_KEY) {
            QFontMetrics nameFontMetrics(debName->font());
            int nameFontSize = nameFontMetrics.width(this->mNameLabel);
            int nameLableWidth = debName->width();
            QString nameFormatBody = this->mNameLabel;
            if (nameFontSize > (nameLableWidth - 10)) {
                nameFormatBody = nameFontMetrics.elidedText(nameFormatBody , Qt::ElideRight , nameLableWidth - 10);
                debName->setFont(this->font);
                debName->setText(nameFormatBody);
                debName->setToolTip(this->mNameLabel);
                this->nameTipStatus = true;
            } else {
                debName->setFont(this->font);
                debName->setToolTip(QString(""));
                this->nameTipStatus = false;
                debName->setText(nameFormatBody);
            }
       }
    });
}
