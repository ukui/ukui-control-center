#include "ukmedia_application_widget.h"
#include <QDebug>
UkmediaApplicationWidget::UkmediaApplicationWidget(QWidget *parent) : QWidget(parent)
{
    m_pApplicationLabel = new QLabel(tr("Application Volume"));
    m_pNoAppLabel = new QLabel(tr("No application is currently playing or recording audio"),this);
    m_pAppWid = new QWidget(this);
    m_pGridlayout = new QGridLayout();
    m_pAppVolumeList = new QStringList;
    m_pApplicationLabel->setFixedSize(160,18);
    m_pNoAppLabel->setFixedSize(300,14);
    m_pNoAppLabel->move(0,30);
    this->setMinimumSize(550,320);
    this->setMaximumSize(960,500);
    m_pAppWid->setMinimumSize(550,320);
    m_pAppWid->setMaximumSize(960,320);

    QVBoxLayout *m_pVlayout = new QVBoxLayout();
    m_pVlayout->addWidget(m_pApplicationLabel);
    m_pVlayout->addWidget(m_pAppWid);
    m_pVlayout->setSpacing(16);
    this->setLayout(m_pVlayout);
    this->layout()->setContentsMargins(0,0,0,0);
   //设置样式
    m_pApplicationLabel->setStyleSheet("QLabel{background:transparent;"
                                       "border:0px;"
                                       "font-weight:500;"
                                       "line-height:40px;"
                                       "font-size:18px;}");
    m_pNoAppLabel->setStyleSheet("QLabel{width:210px;"
                                 "height:14px;"
                                 "font-size:14px;"
                                 "line-height:28px;}");
}

UkmediaApplicationWidget::~UkmediaApplicationWidget()
{

}
