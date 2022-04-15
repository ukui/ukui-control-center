#ifndef PLUGINWIDGET_H
#define PLUGINWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QDBusInterface>
#include <../commonComponent/Label/fixlabel.h>

class PluginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PluginWidget(QStringList List, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    void initUi();
    void setConnect();
    void initStatus();
    void writerFile();
    void tipsDiaolg();
    bool compareStatus();

private:
    QStringList modulesList;
    QListWidget *mListwidget = nullptr;

    FixLabel *mHintLabel = nullptr;

    QPushButton *mCancelBtn = nullptr;
    QPushButton *mConfirmBtn = nullptr;

    QMap <QString, QVariant> mPluginStatus;
    QMap <QString, QVariant> mOriganlStatus;
    QList <bool> isChangeList;

    QDBusInterface *mUkccInterface = nullptr;
    QDBusInterface *mUkccInterface_1 = nullptr;

Q_SIGNALS:
    void close();

};

#endif // PLUGINWIDGET_H
