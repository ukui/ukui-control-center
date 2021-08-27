#ifndef BIOMETRICSWIDGET_H
#define BIOMETRICSWIDGET_H

#include <QWidget>
#include "HoverWidget/hoverwidget.h"
#include "biometricdeviceinfo.h"
#include "biometricproxy.h"
#include "biometricenroll.h"
#include "biometricmoreinfo.h"
#include "pwdchangethread.h"

typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString realname;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
}UserInfomation;

enum {
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};

QT_BEGIN_NAMESPACE
namespace Ui { class BiometricsWidget; }
QT_END_NAMESPACE

class BiometricsWidget : public QWidget
{
    Q_OBJECT

public:
    BiometricsWidget(QWidget *parent = nullptr);
    ~BiometricsWidget();

private:
    Ui::BiometricsWidget *ui;
    //初始化生物特征组件
    void initBioComonent();
    //添加生物特征
    void addFeature(FeatureInfo *featureinfo);
    //更新生物特征设备
    void updateDevice();
    void updateFeatureList();
    void setCurrentDevice(int drvid);
    void setCurrentDevice(const QString &deviceName);
    void setCurrentDevice(const DeviceInfoPtr &pDeviceInfo);
    DeviceInfoPtr findDeviceById(int drvid);
    DeviceInfoPtr findDeviceByName(const QString &name);
    bool deviceExists(int drvid);
    bool deviceExists(const QString &deviceName);
    void showEnrollDialog();
    void showVerifyDialog(FeatureInfo *featureinfo);
    void deleteFeature();
    void deleteFeaturedone(FeatureInfo *feature);
    void renameFeaturedone(FeatureInfo *feature,QString newname);
    void setBiometricDeviceVisible(bool visible);
    void setBioStatus(bool status);
    bool getBioStatus();
    void biometricShowMoreInfoDialog();
    bool isShowBiometric();
    void changeUserPwd(QString pwd, QString username);
    void initUserInfo();
    QString _accountTypeIntToString(int type);

    //增加生物密码
    HoverWidget *addBioFeatureWidget;
    BiometricProxy      *proxy;
    DeviceMap           deviceMap;
    DeviceInfoPtr       currentDevice;
    BiometricProxy      *m_biometricProxy;
    QDBusInterface      *serviceInterface;
    QFileSystemWatcher  *mBiometricWatcher;
    SwitchButton        *enableBiometricBtn;
    QMap<QString, QListWidgetItem *> biometricFeatureMap;
    PwdChangeThread * pcgThread;
    UserInfomation  m_user;

private slots:
    void onbiometricTypeBoxCurrentIndexChanged(int index);
    void onbiometricDeviceBoxCurrentIndexChanged(int index);
    void updateFeatureListCallback(QDBusMessage callbackReply);
    void errorCallback(QDBusError error);
    void showChangePwdDialog();

    /**
     * @brief USB设备热插拔
     * @param drvid     设备id
     * @param action    插拔动作（1：插入，-1：拔出）
     * @param deviceNum 插拔动作后该驱动拥有的设备数量
     */
    void onBiometricUSBDeviceHotPlug(int drvid, int action, int deviceNum);

};
#endif // BIOMETRICSWIDGET_H
