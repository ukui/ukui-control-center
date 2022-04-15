#include "pluginwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDir>
#include <QKeyEvent>
#include <QDialog>
#include <QApplication>
#include "../utils/functionselect.h"
#include "ModuleInfo.h"

PluginWidget::PluginWidget(QStringList List, QWidget *parent) :
    QWidget(parent),
    modulesList(List)
{
    initStatus();
    initUi();
    setConnect();
}

void PluginWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit mConfirmBtn->click();
        return;
    }
    return QWidget::keyPressEvent(event);
}

void PluginWidget::initUi()
{
    setMinimumSize(440, 400);
    QVBoxLayout *Lyt = new QVBoxLayout(this);
    Lyt->setContentsMargins(0, 0, 0, 24);


    mHintLabel = new FixLabel(this);
    mHintLabel->setText(tr("Checked modules will be displayed in the left navigation of ''Settings'', and unchecked modules will be hidden."));

    mListwidget = new QListWidget(this);
    mListwidget->setFrameShape(QListWidget::Box);
    int count = 0;
     for (int moduleIndex = 0; moduleIndex < 8; moduleIndex++){
         QList<FuncInfo> functionStructList = FunctionSelect::funcinfoList[moduleIndex];
         for (int funcIndex = 0; funcIndex < functionStructList.size(); funcIndex++){
             FuncInfo single = functionStructList.at(funcIndex);
             if (modulesList.contains(single.nameString) || modulesList.contains(single.namei18nString)) {
                 QListWidgetItem * item = new QListWidgetItem(mListwidget);
                 item->setSizeHint(QSize(mListwidget->width() - 5, 36));
                 item->setData(Qt::UserRole, "");

                 QFrame *frame = new QFrame(mListwidget);
                 QHBoxLayout *Lyt_2 = new QHBoxLayout(frame);
                 Lyt_2->setContentsMargins(8, 0, 8, 0);
                 QCheckBox *box = new QCheckBox(modulesList.contains(single.nameString) ? single.nameString : single.namei18nString );
                 Lyt_2->addWidget(box);
                 Lyt_2->addStretch();

                 box->setChecked(mPluginStatus.value(single.nameString).toBool());
                 connect(box, &QCheckBox::clicked, [=](){
                    mPluginStatus.insert(single.nameString, box->isChecked());
                    mConfirmBtn->setEnabled(compareStatus());
                 });
                 if (count%2 == 0) {
                     frame->setStyleSheet("QFrame{background: palette(window)}");
                 } else {
                     frame->setStyleSheet("QFrame{background: palette(base)}");
                 }
                 count++;
                 mListwidget->setItemWidget(item, frame);
             }
         }
     }

    QHBoxLayout *Lyt_1 = new QHBoxLayout;
    Lyt_1->setContentsMargins(8, 0, 8, 0);
    Lyt_1->setSpacing(0);
    mCancelBtn = new QPushButton(tr("Cancel"), this);
    mCancelBtn->setFixedWidth(96);
    mConfirmBtn = new QPushButton(tr("Confirm"), this);
    mConfirmBtn->setFixedWidth(96);
    mConfirmBtn->setEnabled(false);


    Lyt_1->addStretch();
    Lyt_1->addWidget(mCancelBtn);
    Lyt_1->addSpacing(10);
    Lyt_1->addWidget(mConfirmBtn);

    Lyt->addWidget(mHintLabel);
    Lyt->addSpacing(8);
    Lyt->addWidget(mListwidget);
    Lyt->addSpacing(24);
    Lyt->addLayout(Lyt_1);

}

void PluginWidget::setConnect()
{
    connect(mCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        writerFile();
       this->close();
        tipsDiaolg();
    });
}

void PluginWidget::initStatus()
{
    mUkccInterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                        "/",
                                                        "com.control.center.interface",
                                                        QDBusConnection::systemBus(), this);
    if (mUkccInterface->isValid()) {
        QDBusMessage result = mUkccInterface->call("getModuleHideStatus");

        QList<QVariant> outArgs = result.arguments();
        QVariant first = outArgs.at(0);
        QDBusArgument dbvFirst = first.value<QDBusArgument>();
        QVariant vFirst = dbvFirst.asVariant();
        const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

        QVector<ModuleInfo> moduleinfo;

        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            ModuleInfo info;
            dbusArgs >> info;
            moduleinfo.push_back(info);
        }
        dbusArgs.endArray();

        for (ModuleInfo it : moduleinfo) {
            mPluginStatus.insert(it.arg, it.out.variant());
            mOriganlStatus.insert(it.arg, it.out.variant());
        }

    }
    mUkccInterface_1 = new QDBusInterface("org.ukui.ukcc.session",
                                          "/",
                                          "org.ukui.ukcc.session.interface",
                                          QDBusConnection::sessionBus(), this);
}

void PluginWidget::writerFile()
{
    if (mUkccInterface->isValid()) {
        QDBusReply<bool> result = mUkccInterface->call("setModuleStatus", mPluginStatus);
        if (result) {
            qDebug()<<"change success";
        } else {
            qDebug()<<"change failed";
        }
    }
}

void PluginWidget::tipsDiaolg()
{
    QDialog *dialog = new QDialog(this);
    dialog->setFixedSize(440, 140);
    QVBoxLayout *Lyt = new QVBoxLayout(dialog);
    Lyt->setContentsMargins(32, 16, 16, 24);

    QHBoxLayout *Lyt_1 = new QHBoxLayout;
    QLabel *textlabel = new QLabel(dialog);
    textlabel->setStyleSheet("QLabel{text-align: left}");
    textlabel->setWordWrap(true);
    textlabel->setText(tr("The module configuration has been modified, the application will restart with the new configuration"));
    Lyt_1->addWidget(textlabel);
    Lyt_1->addStretch();

    QHBoxLayout *Lyt_2 = new QHBoxLayout();
    QPushButton *confirmbtn = new QPushButton(tr("Confirm"), dialog);;
    Lyt_2->addStretch();
    Lyt_2->addWidget(confirmbtn);

    connect(confirmbtn, &QPushButton::clicked, [=](){
        mUkccInterface_1->call("runUkcc");
        qApp->exit(0);
    });

    Lyt->addLayout(Lyt_1);
    Lyt->addStretch();
    Lyt->addLayout(Lyt_2);
    dialog->exec();
}

bool PluginWidget::compareStatus()
{
    QMap<QString, QVariant>::const_iterator iterator = mPluginStatus.constBegin();
    while (iterator != mPluginStatus.constEnd()) {
        if (iterator.value() != mOriganlStatus[iterator.key()]) {
                return true;
                break;
        }
        ++iterator;
    }
    return false;
}
