#include "deleteuserexists.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QButtonGroup>

#include <QDBusInterface>

#include <QDebug>

DeleteUserExists::DeleteUserExists(QString name, QString nick, qint64 uid, QWidget *parent) :
    QDialog(parent),
    _name(name),
    _nick(nick),
    _id(uid)
{
    setFixedSize(QSize(520, 308));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    initUI();
    setConnect();
}

DeleteUserExists::~DeleteUserExists()
{
}

void DeleteUserExists::initUI(){

    removeBtnGroup = new QButtonGroup;

    //标题
    closeBtn = new QPushButton;
    closeBtn->setFixedSize(QSize(14, 14));

    titleHorLayout = new QHBoxLayout;
    titleHorLayout->setSpacing(0);
    titleHorLayout->setMargin(0);
    titleHorLayout->addStretch();
    titleHorLayout->addWidget(closeBtn);

    //提示信息
    QString note1 = tr("Delete user '");
    QString note2 = tr("'? And:");
    noteLabel = new QLabel;
    noteLabel->setFixedHeight(27);
    if (!_nick.isEmpty()){
        noteLabel->setText(note1 + _nick + note2);
    } else {
        noteLabel->setText(note1 + _name + note2);
    }

    noteHorLayout = new QHBoxLayout;
    noteHorLayout->setSpacing(0);
    noteHorLayout->setMargin(0);
    noteHorLayout->addWidget(noteLabel);

    //
    removeButKeepFilesRadioBtn = new QRadioButton;
    removeButKeepFilesRadioBtn->setFixedSize(QSize(16, 16));
    removeWholeRadioBtn = new QRadioButton;
    removeWholeRadioBtn->setFixedSize(QSize(16, 16));

    removeBtnGroup->addButton(removeButKeepFilesRadioBtn, 0);
    removeBtnGroup->addButton(removeWholeRadioBtn, 1);
    removeWholeRadioBtn->setChecked(true);

    removeButKeepFilesLabel = new QLabel;
    removeButKeepFilesLabel->setText(tr("Keep user's data, like desktop,documents,favorites,music,pictures and so on"));
    removeWholeLabel = new QLabel;
    removeWholeLabel->setText(tr("Delete whole data belong user"));

    removeButKeepFilesHorLayout = new QHBoxLayout;
    removeButKeepFilesHorLayout->setSpacing(9);
    removeButKeepFilesHorLayout->setContentsMargins(0, 0, 0, 0);
    removeButKeepFilesHorLayout->addWidget(removeButKeepFilesRadioBtn);
    removeButKeepFilesHorLayout->addWidget(removeButKeepFilesLabel);

    removeWholeHorLayout = new QHBoxLayout;
    removeWholeHorLayout->setSpacing(9);
    removeWholeHorLayout->setContentsMargins(0, 0, 0, 0);
    removeWholeHorLayout->addWidget(removeWholeRadioBtn);
    removeWholeHorLayout->addWidget(removeWholeLabel);

    removeButKeepFilesFrame = new QFrame;
    removeButKeepFilesFrame->setMinimumSize(QSize(472, 60));
    removeButKeepFilesFrame->setMaximumSize(QSize(16777215, 60));
    removeButKeepFilesFrame->setFrameShape(QFrame::StyledPanel);
    removeButKeepFilesFrame->setFrameStyle(QFrame::Plain);
    removeButKeepFilesFrame->setLayout(removeButKeepFilesHorLayout);

    removeWholeFrame = new QFrame;
    removeWholeFrame->setMinimumSize(QSize(472, 60));
    removeWholeFrame->setMaximumSize(QSize(16777215, 60));
    removeWholeFrame->setFrameShape(QFrame::StyledPanel);
    removeWholeFrame->setFrameStyle(QFrame::Plain);
    removeWholeFrame->setLayout(removeWholeHorLayout);

    //底部按钮
    cancelBtn = new QPushButton;
    confirmBtn = new QPushButton;

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 24, 24);
    contentVerLayout->addLayout(noteHorLayout);
    contentVerLayout->addWidget(removeButKeepFilesFrame);
    contentVerLayout->addWidget(removeWholeFrame);
    contentVerLayout->addLayout(bottomBtnsHorLayout);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(0);
    mainVerLayout->setMargin(0);
    mainVerLayout->addLayout(titleHorLayout);
    mainVerLayout->addLayout(contentVerLayout);

    setLayout(mainVerLayout);
}

void DeleteUserExists::setConnect(){

    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        QDBusInterface tmpSysinterface("org.freedesktop.Accounts",
                                       "/org/freedesktop/Accounts",
                                       "org.freedesktop.Accounts",
                                       QDBusConnection::systemBus());

        if (tmpSysinterface.isValid()){
            qDebug() << "call" << "method: deleteuser";
            tmpSysinterface.call("DeleteUser", _id, removeBtnGroup->checkedId() == 1 ? true : false);
        } else {
            qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        }

        close();
    });
}
