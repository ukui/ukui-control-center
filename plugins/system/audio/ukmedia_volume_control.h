#ifndef UKMEDIAVOLUMECONTROL_H
#define UKMEDIAVOLUMECONTROL_H

#include <pulse/ext-stream-restore.h>
#include <signal.h>
#include <string.h>
#include <glib.h>
#include <QObject>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-restore.h>
#include <pulse/ext-device-manager.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QLabel>
#include <QDialog>
#include "ukui_custom_style.h"

#define DECAY_STEP .04

static int n_outstanding = 0;

class PortInfo {
public:
      QByteArray name;
      QByteArray description;
      uint32_t priority;
      int available;
      int direction;
      int64_t latency_offset;
      std::vector<QByteArray> profiles;
};

class UkmediaVolumeControl : public QObject{
    Q_OBJECT
public:
    UkmediaVolumeControl();
    virtual ~UkmediaVolumeControl();

    void updateCard(UkmediaVolumeControl *c,const pa_card_info &info);
    bool updateSink(UkmediaVolumeControl *c,const pa_sink_info &info);
    void updateSource(const pa_source_info &info);
    void updateSinkInput(const pa_sink_input_info &info);
    void updateSourceOutput(const pa_source_output_info &info);
    void updateClient(const pa_client_info &info);
    void updateServer(const pa_server_info &info);
    void updatePorts(UkmediaVolumeControl *d, std::map<QByteArray, PortInfo> &ports);
    void updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v);
//    void updateRole(const pa_ext_stream_restore_info &info);

    void updateDeviceInfo(const pa_ext_device_restore_info &info) ;
    bool setSinkMute(bool status); //设置输出设备静音状态
    bool setSinkVolume(int index,int value); //设置输出设备音量值
    bool setSourceMute(bool status); //设置输入设备静音状态
    bool setSourceVolume(int index,int value); //设置输入设备音量值
    bool setBalanceVolume(int index,int value,float balance); //设置平衡值
    bool getSinkMute(); //获取输出设备静音状态
    int getSinkVolume(); //获取输出设备音量值
    bool getSourceMute(); //获取输入设备静音状态
    int getSourceVolume(); //获取输入设备音量值
    float getBalanceVolume();//获取平衡音量
    int getDefaultSinkIndex();
    int getSinkInputVolume(const gchar *name); //根据name获取sink input的音量值
    void setSinkInputVolume(int index,int value); //设置sink input 音量值
    void setSinkInputMuted(int index,bool status); //设置sink input 静音状态

    int getSourceOutputVolume(const gchar *name); //根据name获取source output的音量值
    bool setCardProfile(int index,const gchar *name); //设置声卡的配置文件
    bool setDefaultSink(const gchar *name); //设置默认的输出设备
    bool setDefaultSource(const gchar *name); //设置默认的输入设备
    bool setSinkPort(const gchar *sinkName ,const gchar *portName); //设置输出设备的端口
    bool setSourcePort(const gchar *sourceName, const gchar *portName); //设置输入设备的端口

    void setSourceOutputVolume(int index, int value);
    void setSourceOutputMuted(int index, bool status);

    void removeCard(uint32_t index);
    void removeSink(uint32_t index);
    void removeSource(uint32_t index);
    void removeSinkInput(uint32_t index);
    void removeSourceOutput(uint32_t index);
    void removeClient(uint32_t index);

    void setConnectingMessage(const char *string = NULL);

    void showError(const char *txt);
    static void decOutstanding(UkmediaVolumeControl *w);
    static void sinkIndexCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void sourceIndexCb(pa_context *c, const pa_source_info *i, int eol, void *userdata);
    static void readCallback(pa_stream *s, size_t length, void *userdata);

    static void cardCb(pa_context *, const pa_card_info *i, int eol, void *userdata);
    static void sinkCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void sourceCb(pa_context *, const pa_source_info *i, int eol, void *userdata);
    static void sinkInputCb(pa_context *, const pa_sink_input_info *i, int eol, void *userdata);
    static void sinkInputCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata); //不更新sink input
    static void sourceOutputCb(pa_context *, const pa_source_output_info *i, int eol, void *userdata);
    static void clientCb(pa_context *, const pa_client_info *i, int eol, void *userdata);
    static void serverInfoCb(pa_context *, const pa_server_info *i, void *userdata);
    static void serverInfoIndexCb(pa_context *, const pa_server_info *i, void *userdata);
    static void extStreamRestoreReadCb(pa_context *,const pa_ext_stream_restore_info *i,int eol,void *userdata);
    static void extStreamRestoreSubscribeCb(pa_context *c, void *userdata);
//    void ext_device_restore_read_cb(pa_context *,const pa_ext_device_restore_info *i,int eol,void *userdata);
//    static void ext_device_restore_subscribe_cb(pa_context *c, pa_device_type_t type, uint32_t idx, void *userdata);
    static void extDeviceManagerReadCb(pa_context *,const pa_ext_device_manager_info *,int eol,void *userdata);
    static void extDeviceManagerSubscribeCb(pa_context *c, void *userdata);
    static void subscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
    static void contextStateCallback(pa_context *c, void *userdata);
    pa_context* getContext(void);
    gboolean connectToPulse(gpointer userdata);

    void updateOutputPortMap();
    void removeOutputPortMap(int index); //移除指定索引的output port
    void removeInputPortMap(int index); //移除指定索引的input port
    void removeCardMap(int index); //移除指定索引的 card
    void removeCardProfileMap(int index); //移除声卡profile map
    void removeSinkPortMap(int index);
    void removeSourcePortMap(int index);
    void removeProfileMap();
    bool isExitOutputPort(QString name);
    void removeInputProfile();
    bool isExitInputPort(QString name);
    QString stringRemoveUnrecignizedChar(QString str);
    void createEventRole(); //控制提示音大小
    void updateRole(const pa_ext_stream_restore_info &info);
    void setExtStreamVolume(int volume, bool state); //设置提示音大小


    std::vector< std::pair<QByteArray,QByteArray> > profiles;
    std::map<QByteArray, PortInfo> ports;
    QByteArray activeProfile;
    QByteArray noInOutProfile;
    QByteArray lastActiveProfile;

    QVector <int> sourceOutputVector; //存储source output索引
    bool hasSinks;
    bool hasSources;
    pa_cvolume m_defaultSinkVolume;

    const pa_sink_info *m_pDefaultSink;
    pa_cvolume defaultInputVolume;
    pa_context* m_pPaContext;
    std::map<uint32_t, char*> clientNames;

    int sinkVolume; //输出音量
    int sourceVolume; //输入音量
    bool sinkMuted; //输出静音状态
    bool sourceMuted; //输入静音状态
    int sinkInputVolume; //sink input 音量
    bool sinkInputMuted; //sink input 静音状态
    float balance; //平衡音量值
    int channel; //通道数
    int inputChannel;
    int sinkInputChannel;
    int sourceOutputChannel;
    QString sinkPortName; //输出设备端口名
    QString sourcePortName; //输入设备端口名
    int defaultOutputCard;
    int defaultInputCard;

    pa_channel_map defaultChannelMap;
    friend class UkmediaMainWidget;

    pa_stream *peak;
    double lastPeak;
    QByteArray name;
    QByteArray description;
    uint32_t index, card_index;
    int peakDetectIndex=-1;
    pa_source_flags  sourceFlags;


    bool offsetButtonEnabled;

    pa_cvolume volume;
    std::vector< std::pair<QByteArray,QByteArray> > dPorts;
    QByteArray activePort;
    QByteArray role;
    QByteArray device;
    pa_ext_stream_restore_info paExtStreamInfo;


    QMap<int, QString> sinkMap; //输出设备
    QMap<int, QString> sourceMap; //输入设备
    QMap<int,QMap<QString,QString>> outputPortMap; //输出端口
    QMap<int,QMap<QString,QString>> inputPortMap; //输入端口
    QMap<int,QMap<QString,QString>> profileNameMap; //声卡输出配置文件
    QMap<int, QMap<QString,QString>>inputPortProfileNameMap; //声卡输入配置文件
    QMap<int, QMap<QString,int>> cardProfilePriorityMap; //记录声卡优先级配置文件
    QMap<int, QList<QString>> cardProfileMap;
    QMap<int, QString> cardMap;
    QMap<int,QMap<QString,QString>> sinkPortMap;
    QMap<int,QMap<QString,QString>> sourcePortMap;
    QMap<int,QString> cardActiveProfileMap;

Q_SIGNALS:
    void paContextReady();
    void updateVolume(int value,bool state);
    void updateSourceVolume(int value,bool state);
    void addSinkInputSignal(const gchar* name,const gchar *id,int index);
    void removeSinkInputSignal(const gchar* name);
    void addSourceOutputSignal(const gchar* name,const gchar *id,int index);
    void removeSourceOutputSignal(const gchar* name);
    void checkDeviceSelectionSianal(const pa_card_info *info);
    void peakChangedSignal(double v);
    void updatePortSignal();
    void updateCboxPortSignal();
    void deviceChangedSignal();


protected Q_SLOTS:
    void timeoutSlot();

public:
    void setConnectionState(gboolean connected);
    void updateDeviceVisibility();
    void reallyUpdateDeviceVisibility();
    pa_stream* createMonitorStreamForSource(uint32_t source_idx, uint32_t stream_idx, bool suspend);


    void setIconFromProplist(QLabel *icon, pa_proplist *l, const char *name);

    pa_context *context;
    QByteArray defaultSinkName, defaultSourceName;

    bool canRenameDevices;
    const pa_server_info *m_pServerInfo;
    const pa_source_info *m_pDefaultSource;
    int sinkIndex;
    int sourceIndex;
    pa_mainloop_api* api;
    QStringList sinkInputList;
    QMap<QString, int> sinkInputMap;
    QMap<QString, int> sourceOutputMap;

private:

    gboolean m_connected;
    gchar* m_config_filename;
};


#endif // UKMEDIAVOLUMECONTROL_H
