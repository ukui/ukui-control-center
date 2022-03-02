#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QFont>
#include <QFile>
#include <QProgressBar>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtDBus/QtDBus>
#include <QFontMetrics>

#include "appupdate.h"
#include "m_updatelog.h"
#include "updatedeleteprompt.h"
#include "updatesource.h"
#include "ukscconn.h"
#include "backup.h"
#include "checkbutton.h"
#include "SwitchButton/switchbutton.h"
#include "Label/titlelabel.h"
#include "Label/fixlabel.h"
//#include <ukcc/widgets/lightlabel.h>

#define CRUCIAL_FILE_PATH "/var/lib/kylin-software-properties/template/crucial.list"
#define IMPORTANT_FIEL_PATH "/var/lib/kylin-software-properties/template/important.list"


const int needBack = 99;

const int netErrorRetry = 3;

enum{
    UpdateAll=1,
    UpdatePart,
    UpdateSystem,
};

enum{
    ACTION_DEFUALT_STATUS=-1,    //默认状态空闲状态
    ACTION_UPDATE,               //处于更新cache状态
    ACTION_INSTALL,              //处于升级安装包括部分升级、全部升级状态
    ACTION_INSTALL_DEB,          //处于安装deb的状态
    ACTION_CHECK_RESOLVER,       //处于计算依赖过程
    ACTION_DOWNLOADONLY,         //单独下载软件包过程
    ACTION_FIX_BROKEN,           //修复依赖的过程
    ACTION_REMOVE_PACKAGES,      //卸载包的状态中

};

class TabWid : public QWidget
{
    Q_OBJECT
public:
    explicit TabWid(QWidget *parent = nullptr);
    ~TabWid();
    void allComponents(); //更新页面里所有组件
    void getpkginfoFromJson(AppAllMsg *msg,QString pkgname);
    void getAllDisplayInformation();
//    void allBinding();  //更新页面里所有组件绑定
    void checkUpdateBtnClicked();  //检测更新、全部更新按钮
    void updateAllApp(bool status);
    void disupdateallaccept();    
    QString modifySpeedUnit(long size, float time);
    QString modifySizeUnit(long size);
    QString getLanguageEnv();
    void showDetails();

//    void updateSettings(); //更新设置页面

    //选项卡页面，具有更新和更新设置两个选项卡及对应界面
    QTabWidget *updateWidget;
    AppUpdateWid *updatewid;
    TitleLabel *labUpdate;
    QWidget *updateTab;
    QWidget *updateSettingTab;
    QVBoxLayout *mainTabLayout;
    QFont ft;
    QCheckBox *isAutoCheckBox;

    //更新页面布局
    QHBoxLayout *tab1HLayout;
    QFrame *systemWidget;
    QVBoxLayout *AppMessage;
    QScrollArea *scrollArea;
    //更新设置页面布局
//    QHBoxLayout *tab2HLayout;
    QVBoxLayout *tab2VLayout;

    //系统头像
    QLabel *systemPortraitLab;
    m_button *checkUpdateBtn;  //检测更新
    //三种状态下的版本信息   显示当前版本、可更新版本、或最新版本
    FixLabel *versionInformationLab;
    QLabel *lastRefreshTime;
    QProgressBar *allProgressBar;
    QLabel *progressLabel;
    QLabel *detailLabel;
    QPushButton *historyUpdateLog;  //历史更新日志界面
    QPushButton *updatedetaileInfo;
    QVBoxLayout *inforLayout;
    QVBoxLayout *controlLayout;

     //更新设置页面组件
    QFrame *updateSettingWidget;
    TitleLabel *updateSettingLab;
    QVBoxLayout *updatesettingLayout;
    QFrame *isAutoCheckWidget;
    QHBoxLayout *isAutoCheckLayout;
    QLabel *isAutoCheckedLab;
    SwitchButton *isAutoCheckSBtn;
    QFrame *isAutoBackupWidget;
    QHBoxLayout *isAutoBackupLayout;
    QLabel *isAutoBackupLab;
    SwitchButton *isAutoBackupSBtn;
    //download limit widgets
    QFrame *DownloadHWidget;
    QFrame *DownloadVWidget;
    //QFrame *isDownloadWidget;
    QHBoxLayout *DownloadHLayout;
    QVBoxLayout *DownloadVLayout;
    QLabel *DownloadHLab;
    FixLabel *DownloadVLab;
    SwitchButton *DownloadHBtn;
    QComboBox *DownloadHValue;
    QHBoxLayout *DownloadVLabLayout;

    QFrame *isAutoUpgradeWidget;
    QVBoxLayout *isAutoUpgradeLayout;
    QHBoxLayout *autoUpgradeLayout;
    QHBoxLayout *autoUpgradeBtnLayout;
    QLabel *isAutoUpgradeLab;
    FixLabel *autoUpgradeLab;
    SwitchButton *isAutoUpgradeSBtn;
    /*二選一*/
    QButtonGroup *m_pButtonGroup;
    QRadioButton *poweroffUpgradeBtn;
    QRadioButton *ontimeUpgradeBtn;
    QComboBox *ontimeUpgradeValue;
    QLabel *ontimeUpgradeLab;
    QHBoxLayout *ontimeUpgradeLayout;

    QWidget *allUpdateWid;
    QVBoxLayout *allUpdateLayout;

    QList<AppUpdateWid *> widgetList;

    bool isAllUpgrade = false;
    bool isAutoUpgrade = false;
    bool isContinueUpgrade = false;
    bool isCancel = false;
    bool alldownloadstart = false;
    int inumber = 0;
    int retryTimes = 0;
    m_updatelog *historyLog;
    updatedeleteprompt *dependsloveptompt;
//    QDialog *historyLog;
    bool fileLockedStatus = false;  //等待静默安装提示是否弹出
    UpdateDbus *updateMutual;
    bool downloadFailedStatus = false;  //下载失败时的弹窗是否弹出
    //源管理器Dbus对象
    UpdateSource *updateSource;

    int allProgress = 0;
    int allUpgradeNum = 0;
    int lastprogress = 0;

    QFileSystemWatcher* filewatcher;

    QList<pkgProgress> pkgList;

signals:
//    void send_Signal();
//    void parameterSignal(int i);
    void updateAllSignal(bool status);
public slots:
    void showHistoryWidget();
    void showDependSlovePtompt(int updatemode, QStringList pkgname,QStringList description,QStringList deletereason);
//    void isAutoCheckedChanged();
//    void isAutoUpgradeChanged();
    void slotCancelDownload();
    void loadingOneUpdateMsgSlot(AppAllMsg msg); //逐个加载更新
    void loadingFinishedSlot(int size); //加载完毕信号
    void waitCrucialInstalled();  //等待静默更新安装完的消息提示

    void hideUpdateBtnSlot(bool state,QStringList pkgname,QString error,QString reason);
    void oneappUpdateresultSlot(bool state,QStringList pkgname,QString error,QString reason);
    void allappupdatefinishSlot();
    void changeUpdateAllSlot(bool isUpdate);
    void updatecancel(void);
    void startoneappupdateslot(void);

//    void DownloadLimitSwitchChanged();
    void DownloadSpeedChange(QString speed);
    void DownloadLimitValueChanged(const QString &);
    void DownloadLimitChanged();
//    void ontimeUpgradeValueChanged(const QString &);
    void getAllProgress (QStringList pkgName,int Progress,QString status,QString detailinfo);
    void showDownloadInfo(QStringList,int,int,uint,uint,int);
    void DependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString error, QString);
    void DistupgradeDependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist,QString error, QString);
    //调用源管理器相关
    void slotUpdateTemplate(QString status);
    void slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason);
    void slotUpdateCacheProgress(int progress, QString status);

    //解决首次运行卡顿问题
    //DBus单独初始化
    void initDbus();

    void slotReconnTimes(int times);

    void isCancelabled(bool status);
    //自动更新相关
    void progresschanged();

private:
    UKSCConn *ukscConnect;
    bool isConnectSourceSignal = false;
    bool distUpgradeAllApp(bool status);
//备份还原相关
    void bacupInit(bool isConnect);
    void backupDelete();
    void backupCore();
    BackUp *backup = nullptr;
    QThread *backupThread = nullptr;

    void backupMessageBox(QString str);
    void backupHideUpdateBtn(int result);

    void getAutoUpgradeStatus();
    bool get_battery();
    bool autoUpdateLoadUpgradeList(bool isBackUp);

    void fileLock();
    void fileUnLock();
//自动更新相关
public Q_SLOTS:
    void autoupgradestatusshow();
    void autoinstallfinish();
    void backupfailure();


signals:
    int needBackUp();
    void startBackUp(int);

public slots:
    void bakeupFinish(int);
    void backupProgress(int);
//    void isAutoBackupChanged();

    void getReplyFalseSlot();
    
    void dbusFinished();

private slots:
    void receiveBackupStartResult(int result);
    void whenStateIsDuing();
};

#endif // TABWIDGET_H
