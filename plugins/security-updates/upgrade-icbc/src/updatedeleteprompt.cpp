#include "updatedeleteprompt.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include "xatom-helper.h"

const QString FIND_DES_LABLE_TYPE = "FIND_DES_LABLE_TYPE";//历史更新模块标签
const QString FIND_REASON_LABLE_TYPE = "FIND_REASON_LABLE_TYPE";//历史更新模块标签
const int WIDTH = 580;//窗口宽度
const int HEIGHT = 610 -40;//窗口高度   注：QFram实际高度比setFixedSize指定的高度高40像素，故减去40
const int LINE_SPACING = 2;//行间距
const int BTN_SPACING = 6;//行间距
const int TOP_MARGIN = 10;//上边距
const int BOTTOM_MARGIN = 24;//下边距
const int LEFT_MARGIN = 32;//左（右）边距

const int LIST_LEFT = 8;//列表对于其背景，左侧边距
const int LIST_TOP = 8;//列表对于其背景，顶（底）部边距
const int LIST_BACKGROUND_WIDTH = 260 + 6;//列表背景宽度
const int SLIDER_WIDTH = 6;//滑块宽度
//const int LIST_WIDTH = LIST_BACKGROUND_WIDTH - LIST_LEFT*2 + SLIDER_WIDTH;//列表宽度
//const int LIST_HEIGHT = 524 - LIST_TOP*2;//列表高度

const int TEXT_TAB_LEFT = 16;//更新详情lable对于其背景，左侧边距
const int TEXT_TAB_RIGHT = 2;//更新详情lable对于其背景，左侧边距
const int TEXT_DSC_LEFT_SPACING = 5;//QTextEdit左侧有边距
const int TEXT_TAB_TOP = 17;//更新详情lable对于其背景，顶部边距
const int TEXT_TAB_SPACING = 18;//更新详情lable对于内容框的间距

updatedeleteprompt * updatedeleteprompt::m_instance(nullptr);

updatedeleteprompt::updatedeleteprompt(QWidget* parent) : QDialog(parent)
{
    /*添加窗管协议*/
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    initUI();//初始化UI
    //initGsettings();//初始化Gsettings
    //dynamicLoadingInit();//动态加载
//    defaultItem();//设置默认选中

    connect(deletepkgdetailbtn,&QPushButton::clicked,this,&updatedeleteprompt::deletepkgdetailbtnclicked);
    connect(deletepkgkeepbtn,&QPushButton::clicked,this,&updatedeleteprompt::deletepkgkeepbtnclicked);
    connect(deletepkgremovebtn,&QPushButton::clicked,this,&updatedeleteprompt::deletepkgremovebtnclicked);

}

updatedeleteprompt * updatedeleteprompt::GetInstance(QWidget *parent)
{
    if(m_instance==nullptr)
    {
        m_instance = new updatedeleteprompt(parent);
        return m_instance;
    }
    if(m_instance->isHidden())
    {
        m_instance->deleteLater();
        m_instance = new updatedeleteprompt(parent);
    }
    return m_instance;
}

void updatedeleteprompt::initUI()
{
    QFont font;
    font.setBold(true);

    /* 初始化窗口属性 */
    this->setFixedSize(WIDTH,HEIGHT + 40);
    this->setObjectName(FIND_DES_LABLE_TYPE);

    /* 实例化控件 */
    /* 实例化标题栏 */
    updateTitleWidget();

    conflictpromptTab = new QLabel;
    conflictpromptTab->setFont(font);
    conflictpromptTab->setWordWrap(true);
    conflictpromptTab->setText(tr("Dependency conflict exists in this update!"));
    conflictpromptTab->setStyleSheet("font-size:22px;font-weight:600;line-height: 28px;");

    uninstallpromptTab = new QLabel;
//    uninstallpromptTab->setFont(font);
    uninstallpromptTab->setWordWrap(true);
    uninstallpromptTab->setText(tr("There will be uninstall some packages to complete the update!"));
    uninstallpromptTab->setStyleSheet("font-size:16px;font-weight:500;line-height: 22px;");

    uninstallinfoTab = new QLabel;
    uninstallinfoTab->setWordWrap(true);
    uninstallinfoTab->setStyleSheet("font-size:16px;font-weight:500;line-height: 22px;");

    uninstallDesTab = new QLabel;
    uninstallDesTab->setWordWrap(true);
    uninstallDesTab->setText(tr("The following packages will be uninstalled:"));
    uninstallDesTab->setStyleSheet("font-size:15px;font-weight:500;line-height: 21px;");
    uninstallDesTab->hide();

    listBackground = new QFrame;
    listBackground->setFrameStyle(QFrame::Box);
    listBackground->setFixedWidth(LIST_BACKGROUND_WIDTH);
    listBackground->hide();

    mainListwidget  = new QListWidget;
    QPalette palette = mainListwidget->palette();
    palette.setBrush(QPalette::Base, QColor (0, 0 , 0, 0));
    mainListwidget->setPalette(palette);
    mainListwidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);  /* 滑块平滑滚动 */
    mainListwidget->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
    mainListwidget->setSpacing(2);
    mainListwidget->hide();

    desBackground = new QFrame;
    desBackground->setFrameStyle(QFrame::Box);
    desBackground->hide();

    updateDesTab = new QLabel;
//    updateDesTab->setFont(font);
    updateDesTab->setWordWrap(true);
    updateDesTab->setText(tr("PKG Details"));  /* 更新详情 */
    updateDesTab->hide();

    deleteReasonTab = new QLabel;
    deleteReasonTab->setFont(font);
    deleteReasonTab->setWordWrap(true);
    deleteReasonTab->setObjectName(FIND_REASON_LABLE_TYPE);
    deleteReasonTab->hide();

    deletepkgdetailbtn = new QPushButton;
    deletepkgdetailbtn->setText(tr("details"));
    deletepkgdetailbtn->setFixedSize(100,35);

    deletepkgkeepbtn = new QPushButton;
    deletepkgkeepbtn->setText(tr("Keep"));
    deletepkgkeepbtn->setFixedSize(100,35);

    deletepkgremovebtn = new QPushButton;
    deletepkgremovebtn->setText(tr("Remove"));
    deletepkgremovebtn->setFixedSize(100,35);

    /* 描述文本框 */
    des = new QTextEdit;
    QPalette palette2 = des->palette();
    palette2.setBrush(QPalette::Base, QColor (0, 0 , 0, 0));
    des->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
    des->setPalette(palette2);
    des->setReadOnly(true);
    des->setObjectName(FIND_DES_LABLE_TYPE);
    des->hide();

    /* 布局 */
    /* 布局外边框 */

    QVBoxLayout *vu1 = new QVBoxLayout;
    vu1->setSpacing(0);
    vu1->setMargin(0);
    vu1->addSpacing(LINE_SPACING);
    vu1->addWidget(conflictpromptTab,Qt::AlignLeft | Qt::AlignTop);
    vu1->addSpacing(LINE_SPACING);
    vu1->addWidget(uninstallpromptTab,Qt::AlignLeft | Qt::AlignTop);
    vu1->addSpacing(LINE_SPACING);
    vu1->addWidget(uninstallinfoTab,Qt::AlignLeft | Qt::AlignTop);
    vu1->addSpacing(LINE_SPACING);

    QWidget *conflictdeswgt = new QWidget(this);
    conflictdeswgt->setLayout(vu1);

    QHBoxLayout *hw1 = new QHBoxLayout;
    hw1->addWidget(deletepkgdetailbtn,Qt::AlignLeft);
    hw1->addStretch(0);

    QVBoxLayout *vw1 = new QVBoxLayout;
    vw1->setSpacing(0);
    vw1->setMargin(0);
    vw1->addSpacing(TOP_MARGIN);
    vw1->addWidget(conflictdeswgt);
    vw1->addSpacing(LINE_SPACING);
    vw1->addLayout(hw1);
    vw1->addSpacing(TEXT_DSC_LEFT_SPACING);
    vw1->addWidget(uninstallDesTab);

    QHBoxLayout *hu1 = new QHBoxLayout;
    hu1->setSpacing(0);
    hu1->setMargin(0);
    hu1->addSpacing(LEFT_MARGIN);
    hu1->addLayout(vw1);
    hu1->addSpacing(LEFT_MARGIN);

    QHBoxLayout *hl1 = new QHBoxLayout;
    hl1->setSpacing(0);
    hl1->setMargin(0);
    hl1->addSpacing(LEFT_MARGIN);
    hl1->addWidget(listBackground);
    hl1->addSpacing(LINE_SPACING);
    hl1->addWidget(desBackground);
    hl1->addSpacing(LEFT_MARGIN);

    QWidget *conflictpkgwgt = new QWidget(this);
    conflictpkgwgt->setFixedHeight(260);
    conflictpkgwgt->setLayout(hl1);

    QHBoxLayout *hs1 = new QHBoxLayout;
    hs1->setSpacing(0);
    hs1->setMargin(0);
    hs1->addStretch(0);
    hs1->addWidget(deletepkgkeepbtn,Qt::AlignRight);
    hs1->addSpacing(BTN_SPACING);
    hs1->addWidget(deletepkgremovebtn,Qt::AlignRight);
    hs1->addSpacing(LEFT_MARGIN);

    QVBoxLayout *vl1 = new QVBoxLayout;
    vl1->setSpacing(0);
    vl1->setMargin(0);

    vl1->addSpacing(6);
    vl1->addWidget(this->title);

    vl1->addSpacing(TOP_MARGIN);
    vl1->addLayout(hu1);
    vl1->addSpacing(TOP_MARGIN);
    vl1->addWidget(conflictpkgwgt);
    vl1->addSpacing(TOP_MARGIN);
    vl1->addLayout(hs1);
    vl1->addSpacing(BOTTOM_MARGIN);
    this->setLayout(vl1);

    /* 布局列表 */
    hll = new QHBoxLayout;
    hll->setSpacing(0);
    hll->setMargin(0);
    hll->addSpacing(LIST_LEFT);
    hll->addWidget(mainListwidget);

    QVBoxLayout *vll = new QVBoxLayout;
    vll->setSpacing(0);
    vll->setMargin(0);
    vll->addSpacing(LIST_TOP);
    vll->addLayout(hll);
    vll->addSpacing(LIST_TOP);
    listBackground->setLayout(vll);

    /* 布局更新详情标签 */
    QHBoxLayout *hlrt = new QHBoxLayout;
    hlrt->setSpacing(0);
    hlrt->setMargin(0);
    hlrt->addSpacing(TEXT_DSC_LEFT_SPACING);
    hlrt->addWidget(updateDesTab);

    /* 布局删包原因标签 */
    QHBoxLayout *hlst = new QHBoxLayout;
    hlst->setSpacing(0);
    hlst->setMargin(0);
    hlst->addSpacing(TEXT_DSC_LEFT_SPACING);
    hlst->addWidget(deleteReasonTab);

    /* 布局标签及文本框 */
    QVBoxLayout *vlr = new QVBoxLayout;
    vlr->setSpacing(0);
    vlr->setMargin(0);
    vlr->addSpacing(TEXT_TAB_TOP);
    vlr->addLayout(hlrt);
    vlr->addSpacing(TEXT_TAB_SPACING);
    vlr->addLayout(hlst);
    vlr->addSpacing(TEXT_TAB_SPACING);
    vlr->addWidget(des);
    vlr->addSpacing(TEXT_TAB_TOP);

    QHBoxLayout *hlr = new QHBoxLayout;
    hlr->setSpacing(0);
    hlr->setMargin(0);
    hlr->addSpacing(TEXT_TAB_LEFT - TEXT_DSC_LEFT_SPACING);
    hlr->addLayout(vlr);
    hlr->addSpacing(TEXT_TAB_RIGHT);
    desBackground->setLayout(hlr);
}

void updatedeleteprompt::updateTitleWidget(void)
{
    this->title = new QWidget(this);
    this->title->setFixedHeight(36);

    /* 标题栏图标 */
    this->titleIcon = new QLabel(this);
    this->titleIcon->setFixedSize(25 , 25);
    this->titleIcon->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(QSize(25 , 25)));

    /* 标题栏名字 */
    this->titleName = new QLabel(this);
    this->titleName->resize(56 , 20);
    QFont font;
    font.setPixelSize(14);
    this->titleName->setFont(font);
    //this->titleName->setText(tr("更新提醒"));
    this->titleName->setText(tr("Update Prompt"));

    /* 关闭按钮 */
    this->titleClose = new QPushButton(this);
    this->titleClose->setFixedSize(30 , 30);
    this->titleClose->setFlat(true);
    this->titleClose->setProperty("isWindowButton" , 0x2);
    this->titleClose->setProperty("useIconHighlightEffect" , 0x8);
    this->titleClose->setIconSize(QSize(16 , 16));
    this->titleClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    this->titleClose->setFocusPolicy(Qt::NoFocus);

    connect(this->titleClose , &QPushButton::clicked , this , &updatedeleteprompt::slotClose);

    /* 布局 */
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->addSpacing(8);
    hlayout->addWidget(this->titleIcon);
    hlayout->addSpacing(2);
    hlayout->addWidget(this->titleName);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->setMargin(0);
    hlayout2->addLayout(hlayout);
    hlayout2->addStretch(0);
    hlayout2->addWidget(this->titleClose);
    hlayout2->addSpacing(6);

    this->title->setLayout(hlayout2);
}

int updatedeleteprompt::updatedeletepkglist(QStringList pkgname,QStringList description,QStringList deletereason)
{
    DeletePkgListWig *firstWidget = NULL;
    bool flag = true;
    int size=0;

    QList<QString>::Iterator it = pkgname.begin(),itend = pkgname.end();
    int i = 0;
    for (;it != itend; it++,i++){
        qDebug() << "updatedeletepkglist函数：获取到的包列表：" << pkgname[i];
        size++;

        DeletePkgListWig *hulw = new DeletePkgListWig(updateDesTab);
        if (flag == true) {
            firstWidget = hulw;
            flag = false;
        }
        hulw->setAttribute(pkgname[i],description[i],deletereason[i]);
        QListWidgetItem *item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(hulw->getTrueSize());
        mainListwidget->addItem(item);
        mainListwidget->setItemWidget(item,hulw);
        if(pkgname[i]!="")
            hulw->selectStyle();//设置选中样式
    }

    /* 选中第一条记录 */
    if (firstWidget != NULL) {
        firstWidget->selectStyle();
    }
    return size;
}

void updatedeleteprompt::defaultItem()
{
    //默认选中第一个
    DeletePkgListWig *first = mainListwidget->findChild<DeletePkgListWig *>();
    if(first!=nullptr)
    {
        first->selectStyle();//设置选中样式
        firstCode = first->id;//记录id
    }
}

void updatedeleteprompt::deletepkgdetailbtnclicked()
{
    if(mainListwidget->isHidden())
    {
        mainListwidget->show();
        desBackground->show();
        listBackground->show();
        updateDesTab->show();
        des->show();
        deleteReasonTab->show();
//        uninstallDesTab->show();
        //        updatedetaileInfo->setText(tr("收起"));
        deletepkgdetailbtn->setText(tr("back"));
    }
    else
    {
        mainListwidget->hide();
        desBackground->hide();
        listBackground->hide();
        updateDesTab->hide();
        des->hide();
        deleteReasonTab->hide();
//        uninstallDesTab->hide();
        //        updatedetaileInfo->setText(tr("详情"));
        deletepkgdetailbtn->setText(tr("details"));
    }

}

void updatedeleteprompt::deletepkgkeepbtnclicked(void)
{
    this->close();
    emit updatedependsolvecancelsignal();
}

void updatedeleteprompt::deletepkgremovebtnclicked(void)
{
    UpdateDbus *uddbus = UpdateDbus::getInstance();
    this->close();
    if(updatemode==updateAll){
        updatemode=0;
//        uddbus->DistUpgradeAll(true);
        emit updatealldependsolveacceptsignal(true);
    }else if(updatemode==updatePart){
        updatemode=0;
        emit updatedependsolveacceptsignal();
    }else if(updatemode==updateSystem){
        updatemode=0;
//        uddbus->DistUpgradeSystem(true);
        emit disupdatedependsolveacceptsignal();
    }

}

void updatedeleteprompt::slotClose(void)
{
    this->close();
    emit updatedependsolvecancelsignal();
}
