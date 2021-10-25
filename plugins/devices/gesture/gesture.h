#ifndef GESTURE_H
#define GESTURE_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include "shell/interface.h"
#include "Label/titlelabel.h"
#include "itemwidget.h"
#include "FlowLayout/flowlayout.h"
#include <QDebug>
namespace Ui {
class Gesture;
}

class Gesture : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)


public:
    explicit Gesture();
    ~Gesture();
public:
    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;


    void init();
    void additem();
private:
    Ui::Gesture *ui;
    QWidget * pluginWidget=nullptr;

    QString pluginName;
    int pluginType;
    TitleLabel * currentLabel;
private:
    bool mFirstLoad;
    QSettings *gesturesetting;
    FlowLayout * gifFlowLayout;
};

#endif // GESTURE_H
