#include "trialdialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>


TrialDialog::TrialDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set"));
    initUi(this);
}

TrialDialog::~TrialDialog()
{

}

void TrialDialog::initUi(QDialog *mTrialDialog)
{
    mTrialDialog->setFixedSize(560,560);

    QVBoxLayout *mverticalLayout = new QVBoxLayout(mTrialDialog);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(32, 32, 32, 24);

    QHBoxLayout *mTitleLayout = new QHBoxLayout;


    mTitleLabel = new TitleLabel(mTrialDialog);
    mTitleLabel->setFixedHeight(30);
    mTitleLayout->addStretch();
    mTitleLayout->addWidget(mTitleLabel);
    mTitleLayout->addStretch();
    mTitleLabel->setText(tr("Yinhe Kylin OS(Trail Version) Disclaimer"));
    mverticalLayout->addLayout(mTitleLayout,Qt::AlignTop);
    mverticalLayout->addSpacing(24);

    QVBoxLayout *mContentLayout = new QVBoxLayout;
    mContentLayout->setContentsMargins(0, 0, 0, 0);

    mContentLabel_2 = new QTextBrowser(mTrialDialog);
    mContentLabel_2->setFixedHeight(364);
    mContentLabel_2->setText(tr("Dear customer:\n"
                                "       Thank you for trying Yinhe Kylin OS(trail version)! This version is free for users who only try out,"
                                " no commercial purpose is permitted. The trail period lasts one year and it starts from the ex-warehouse time of the OS. No after-sales"
                                " service is provided during the trail stage. If any security problems occurred when user put important files or do any commercial usage"
                                " in system, all consequences are taken by users. Kylin software Co., Ltd. take no legal risk in trail version.\n"
                                "       During trail stage,if you want any technology surpport or activate the system, please buy“Yinhe Kylin Operating System”official "
                                "version or authorization by contacting 400-089-1870."));
    mContentLabel_2->adjustSize();
    mContentLayout->addWidget(mContentLabel_2);
    mContentLayout->addStretch();

    QHBoxLayout *mContentLayout_1 = new QHBoxLayout;

    mContentLabel_4 = new QLabel(mTrialDialog);
    mContentLabel_4->setText(tr("Kylin software Co., Ltd."));
    mContentLayout_1->addStretch();
    mContentLayout_1->addWidget(mContentLabel_4);
    mContentLayout->addLayout(mContentLayout_1);

    QHBoxLayout *mContentLayout_2 = new QHBoxLayout;

    mContentLabel_5 = new QLabel(mTrialDialog);
    mContentLabel_5->setText(tr("www.Kylinos.cn"));
    mContentLayout_2->addStretch();
    mContentLayout_2->addWidget(mContentLabel_5);
    mContentLayout->addLayout(mContentLayout_2);


    mverticalLayout->addLayout(mContentLayout);
}
