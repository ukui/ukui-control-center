#ifndef UKCCPLUGIN_H
#define UKCCPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QTranslator>
#include <QIcon>
#include <QLocale>
#include <QApplication>

#include <ukcc/interface/interface.h>

class UkccPlugin : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:

    UkccPlugin();
    ~UkccPlugin();

    QString plugini18nName()        Q_DECL_OVERRIDE;
    int pluginTypes()               Q_DECL_OVERRIDE;
    QWidget *pluginUi()             Q_DECL_OVERRIDE;
    bool isEnable() const           Q_DECL_OVERRIDE;
    const QString name() const      Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const   Q_DECL_OVERRIDE;
    QIcon icon() const              Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    QWidget *widget;

private:
    void initSearchText();      // 搜索翻译
};

#endif // UKCCPLUGIN_H
