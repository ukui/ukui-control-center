#include "ukmedia_volume_control.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <set>

#include <pulse/introspect.h>
#include <QIcon>
#include <QStyle>
#include <QLabel>
#include <QSettings>
#include <QDebug>
#include <QSlider>
#include <QTimer>
//pa_sink_info *m_pDefaultSink;
/* Used for profile sorting */

int reconnect_timeout;
struct profile_prio_compare {
    bool operator() (pa_card_profile_info2 const * const lhs, pa_card_profile_info2 const * const rhs) const {

        if (lhs->priority == rhs->priority)
            return strcmp(lhs->name, rhs->name) > 0;

        return lhs->priority > rhs->priority;
    }
};

struct sink_port_prio_compare {
    bool operator() (const pa_sink_port_info& lhs, const pa_sink_port_info& rhs) const {

        if (lhs.priority == rhs.priority)
            return strcmp(lhs.name, rhs.name) > 0;

        return lhs.priority > rhs.priority;
    }
};

struct source_port_prio_compare {
    bool operator() (const pa_source_port_info& lhs, const pa_source_port_info& rhs) const {

        if (lhs.priority == rhs.priority)
            return strcmp(lhs.name, rhs.name) > 0;

        return lhs.priority > rhs.priority;
    }
};

UkmediaVolumeControl::UkmediaVolumeControl():
    canRenameDevices(false),
    m_connected(false),
    m_config_filename(nullptr) {

    profileNameMap.clear();
    connectToPulse(this);
}

/*
 *  设置输出设备静音
 */
bool UkmediaVolumeControl::setSinkMute(bool status)
{
    pa_operation* o;

    if (!(o = pa_context_set_sink_mute_by_index(getContext(), sinkIndex, status, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 *  设置输出设备音量
 */
bool UkmediaVolumeControl::setSinkVolume(int index,int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = channel;

    for (int i=0;i<v.channels;i++)
        v.values[i] = value;
    if (balance != 0) {
        qDebug() << "pa_cvolume_set_balance" <<balance;
        pa_cvolume_set_balance(&v,&defaultChannelMap,balance);
    }

    qDebug() << "set sink volume"  << sinkIndex << v.channels << "balance:" << balance;
    pa_operation* o;
    if (sinkMuted) {
        if (!(o = pa_context_set_sink_mute_by_index(getContext(), index,false, nullptr, nullptr))) {
            showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        }
    }
    if (!(o = pa_context_set_sink_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 *  设置输入设备静音
 */
bool UkmediaVolumeControl::setSourceMute(bool status)
{
    pa_operation* o;

    if (!(o = pa_context_set_source_mute_by_index(getContext(), sourceIndex, status, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}


/*
 * 设置输入设备音量
 */
bool UkmediaVolumeControl::setSourceVolume(int index,int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = 2;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set source volume" << sourceIndex << v.channels << pa_context_get_state(context) << "context index:" << pa_context_get_state(getContext());
    pa_operation* o;
    if (sourceMuted) {
        if (!(o = pa_context_set_source_mute_by_index(getContext(), index,false, nullptr, nullptr))) {
            showError(tr("pa_context_set_source_mute_by_index() failed").toUtf8().constData());
        }
    }

    if (!(o = pa_context_set_source_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置音量平衡值
 */
bool UkmediaVolumeControl::setBalanceVolume(int index, int value, float b)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = channel;

    for (int i=0;i<v.channels;i++)
        v.values[i] = value;
    if (b != 0) {
        balance = b;
        qDebug() << "pa_cvolume_set_balance" <<balance;
        pa_cvolume_set_balance(&v,&defaultChannelMap,balance);
    }

    qDebug() << "set balance volume"  << sinkIndex << v.channels << "balance:" << balance;
    pa_operation* o;

    if (!(o = pa_context_set_sink_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 获取输出设备的静音状态
 */
bool UkmediaVolumeControl::getSinkMute()
{
    return sinkMuted;
}

/*
 * 获取输出设备的音量值
 */
int UkmediaVolumeControl::getSinkVolume()
{
    return sinkVolume;
}

/*
 * 获取平衡音量
 */
float UkmediaVolumeControl::getBalanceVolume()
{
    return balance;
}

/*
 * 获取输入设备的静音状态
 */
bool UkmediaVolumeControl::getSourceMute()
{
    return sourceMuted;
}

/*
 * 获取输入设备的音量值
 */
int UkmediaVolumeControl::getSourceVolume()
{
    return sourceVolume;
}

int UkmediaVolumeControl::getDefaultSinkIndex()
{
    pa_operation *o;
    if (!(o = pa_context_get_server_info(getContext(), serverInfoCb, this))) {
        showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
        return -1;
    }
    pa_operation_unref(o);
    qDebug() << "getDefaultSinkIndex" << defaultSinkName << defaultSourceName << sinkVolume;
    return sinkIndex;
}

/*
 *  滑动条更改设置sink input 音量值
 */
void UkmediaVolumeControl::setSinkInputVolume(int index, int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = 2;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set sink input volume" << index << v.channels << value;
    pa_operation* o;
    if (!(o = pa_context_set_sink_input_mute(getContext(), index,false, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }

    if (!(o = pa_context_set_sink_input_volume(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置sink input静音状态
 */
void UkmediaVolumeControl::setSinkInputMuted(int index, bool status)
{
    qDebug() << "set sink input muted" << index << status;
    pa_operation* o;
    if (!(o = pa_context_set_sink_input_mute(getContext(), index,status, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置source output 音量值
 */
void UkmediaVolumeControl::setSourceOutputVolume(int index, int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = 2;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set source output volume" << index << v.channels << value;
    pa_operation* o;
    if (!(o = pa_context_set_source_output_mute(getContext(), index,false, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_volume() failed").toUtf8().constData());
    }

    if (!(o = pa_context_set_source_output_volume(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_volume() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置source output静音状态
 */
void UkmediaVolumeControl::setSourceOutputMuted(int index, bool status)
{
    qDebug() << "set source output muted" << index << status;
    pa_operation* o;
    if (!(o = pa_context_set_source_output_mute(getContext(), index,status, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_mute() failed").toUtf8().constData());
    }
}

/*
 * 设置声卡的配置文件
 */
bool UkmediaVolumeControl::setCardProfile(int index, const gchar *name)
{
    qDebug() << "setCardProfile" << index << name;
    pa_operation* o;
    if (!(o = pa_context_set_card_profile_by_index(getContext(), index, name, nullptr, nullptr))) {
        showError(tr("pa_context_set_card_profile_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置默认的输出设备
 */
bool UkmediaVolumeControl::setDefaultSink(const gchar *name)
{
    qDebug() << "setDefaultSink" << name;
    pa_operation* o;
    if (!(o = pa_context_set_default_sink(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_sink() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置默认的输入设备
 */
bool UkmediaVolumeControl::setDefaultSource(const gchar *name)
{
    qDebug() << "setDefaultSource" << name;
    pa_operation* o;
    if (!(o = pa_context_set_default_source(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_source() failed").toUtf8().constData());
        return false;
    }

    if(!(o = pa_context_get_source_info_by_name(getContext(),defaultSourceName,sourceIndexCb,this))) {
        showError(tr("pa_context_get_source_info_by_name() failed").toUtf8().constData());
    }
    pa_operation_unref(o);
    if (!sourceOutputVector.contains(sourceIndex) && pa_context_get_server_protocol_version(getContext()) >= 13) {
//        sourceOutputVector.append(info.index);
        pa_operation* o;
        qDebug() <<"killall source output index" <<peakDetectIndex;
        if (!(o = pa_context_kill_source_output(getContext(), peakDetectIndex, nullptr, nullptr))) {
            showError(tr("pa_context_set_default_source() failed").toUtf8().constData());
            return false;
        }
        sourceOutputVector.removeAt(0);
        QTimer::singleShot( 100, this,[=](){
            sourceOutputVector.append(sourceIndex);
            peak = createMonitorStreamForSource(sourceIndex, -1, !!(sourceFlags & PA_SOURCE_NETWORK));
        });
    }
    return true;
}

/*
 * 设置输出设备的端口
 */
bool UkmediaVolumeControl::setSinkPort(const gchar *sinkName, const gchar *portName)
{
    qDebug() << "setSinkPort" << sinkName << portName;
    pa_operation* o;
    if (!(o = pa_context_set_sink_port_by_name(getContext(), sinkName, portName, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_port_by_name() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置输入设备端口
 */
bool UkmediaVolumeControl::setSourcePort(const gchar *sourceName, const gchar *portName)
{
    qDebug() << "setSourcePort" << sourceName << portName;
    sourcePortName = portName;
    pa_operation* o;
    if (!(o = pa_context_set_source_port_by_name(getContext(), sourceName, portName, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_port_by_name() failed").toUtf8().constData());
        return false;
    }
    return true;
}

UkmediaVolumeControl::~UkmediaVolumeControl() {

    while (!clientNames.empty()) {
        auto i = clientNames.begin();
        g_free(i->second);
        clientNames.erase(i);
    }
}

static void updatePorts(UkmediaVolumeControl *d, std::map<QByteArray, PortInfo> &ports) {
    std::map<QByteArray, PortInfo>::iterator it;
    PortInfo p;
    for (auto & port : d->dPorts) {
        QByteArray desc;
        it = ports.find(port.first);

        if (it == ports.end())
            continue;

        p = it->second;
        desc = p.description;

        if (p.available == PA_PORT_AVAILABLE_YES)
            desc +=  UkmediaVolumeControl::tr(" (plugged in)").toUtf8().constData();
        else if (p.available == PA_PORT_AVAILABLE_NO) {
            if (p.name == "analog-output-speaker" ||
                p.name == "analog-input-microphone-internal")
                desc += UkmediaVolumeControl::tr(" (unavailable)").toUtf8().constData();
            else
                desc += UkmediaVolumeControl::tr(" (unplugged)").toUtf8().constData();
        }

        port.second = desc;
        qDebug() << "updatePorts" << p.name << p.description;
    }

    Q_EMIT d->updatePortSignal();
    it = ports.find(d->activePort);

    if (it != ports.end()) {
        p = it->second;
//        d->setLatencyOffset(p.latency_offset);
    }
}

static void setIconByName(QLabel* label, const char* name) {
    QIcon icon = QIcon::fromTheme(name);
    int size = label->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QPixmap pix = icon.pixmap(size, size);
    label->setPixmap(pix);
}

void UkmediaVolumeControl::updateCard(UkmediaVolumeControl *c, const pa_card_info &info) {

    bool is_new = false;
    const char *description;
    QMap<QString,QString> tempInput;
    QMap<QString,QString> tempOutput;
    QList<QString> profileName;
    QMap<QString,QString>portMap;
    QMap<QString,QString>inputPortNameLabelMap;
    QMap<QString,int> profilePriorityMap;
    std::set<pa_card_profile_info2 *, profile_prio_compare> profile_priorities;

    description = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_DESCRIPTION);

    hasSinks = c->hasSources = false;
    profile_priorities.clear();
    for (pa_card_profile_info2 ** p_profile = info.profiles2; *p_profile != nullptr; ++p_profile) {
//        c->hasSinks = c->hasSinks || ((*p_profile)->n_sinks > 0);
//        c->hasSources = c->hasSources || ((*p_profile)->n_sources > 0);
        profile_priorities.insert(*p_profile);
        profileName.append((*p_profile)->name);
        profilePriorityMap.insertMulti((*p_profile)->name,(*p_profile)->priority);
    }
    cardProfilePriorityMap.insertMulti(info.index,profilePriorityMap);

    c->ports.clear();
    for (uint32_t i = 0; i < info.n_ports; ++i) {
        PortInfo p;

        p.name = info.ports[i]->name;
        p.description = info.ports[i]->description;
        p.priority = info.ports[i]->priority;
        p.available = info.ports[i]->available;
        p.direction = info.ports[i]->direction;
        p.latency_offset = info.ports[i]->latency_offset;
        if (info.ports[i]->profiles2)
            for (pa_card_profile_info2 ** p_profile = info.ports[i]->profiles2; *p_profile != nullptr; ++p_profile) {
                p.profiles.push_back((*p_profile)->name);
            }
        if (p.direction == 1 && p.available != PA_PORT_AVAILABLE_NO) {
//            portMap.insertMulti(p.name,p.description.data());
            qDebug() << " add sink port name "<< info.index << p.name << p.description.data();
            tempOutput.insertMulti(p.name,p.description.data());

            QList<QString> portProfileName;
            for (auto p_profile : p.profiles) {
                portProfileName.append(p_profile.data());
                QString portName = p.description.data();
                QString profileName = p_profile.data();
                profileNameMap.insertMulti(portName,profileName);
                qDebug() << "ctf profilename map insert -----------" << p.description.data() << p_profile.data();
            }
            cardProfileMap.insertMulti(info.index,portProfileName);
        }
        else if (p.direction == 2 && p.available != PA_PORT_AVAILABLE_NO){

            qDebug() << " add source port name "<< info.index << p.name << p.description.data();
            tempInput.insertMulti(p.name,p.description.data());
            for (auto p_profile : p.profiles) {
                inputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
            }
            inputPortProfileNameMap.insert(info.index,inputPortNameLabelMap);
        }
        c->ports[p.name] = p;
    }
    inputPortMap.insert(info.index,tempInput);
    outputPortMap.insert(info.index,tempOutput);
    cardActiveProfileMap.insert(info.index,info.active_profile->name);
    c->profiles.clear();
    for (auto p_profile : profile_priorities) {
        bool hasNo = false, hasOther = false;
        std::map<QByteArray, PortInfo>::iterator portIt;
        QByteArray desc = p_profile->description;

        for (portIt = c->ports.begin(); portIt != c->ports.end(); portIt++) {
            PortInfo port = portIt->second;

            if (std::find(port.profiles.begin(), port.profiles.end(), p_profile->name) == port.profiles.end())
                continue;

            if (port.available == PA_PORT_AVAILABLE_NO)
                hasNo = true;
            else {
                hasOther = true;
                break;
            }
        }
        if (hasNo && !hasOther)
            desc += tr(" (unplugged)").toUtf8().constData();

        if (!p_profile->available)
            desc += tr(" (unavailable)").toUtf8().constData();

        c->profiles.push_back(std::pair<QByteArray,QByteArray>(p_profile->name, desc));
        if (p_profile->n_sinks == 0 && p_profile->n_sources == 0)
            c->noInOutProfile = p_profile->name;
    }
    c->activeProfile = info.active_profile ? info.active_profile->name : "";

    /* Because the port info for sinks and sources is discontinued we need
     * to update the port info for them here. */
    updatePorts(c,c->ports);
    if (is_new)
        updateDeviceVisibility();

    Q_EMIT checkDeviceSelectionSianal(&info);
//    c->updating = false;
}

/*
 * Update output device when the default output device or port is updated
 */
bool UkmediaVolumeControl::updateSink(UkmediaVolumeControl *w,const pa_sink_info &info) {
    bool is_new = false;
    m_defaultSinkVolume = info.volume;
    QMap<QString,QString>temp;

    //默认的输出音量
    if (info.name && strcmp(defaultSinkName.data(),info.name) == 0) {

        sinkIndex= info.index;
        int volume;
        channel = info.volume.channels;
        if (info.volume.channels >= 2)
            volume = MAX(info.volume.values[0],info.volume.values[1]);
        else
            volume = info.volume.values[0];
        balance = pa_cvolume_get_balance(&info.volume,&info.channel_map);
        defaultChannelMap = info.channel_map;
        channelMap = info.channel_map;
        if (info.active_port) {
            if (strcmp(sinkPortName.toLatin1().data(),info.active_port->name) != 0) {
                sinkPortName = info.active_port->name;
                QTimer::singleShot(50, this, SLOT(timeoutSlot()));
            }
            else
                sinkPortName = info.active_port->name;
        }
        defaultOutputCard = info.card;
        if (sinkVolume != volume || sinkMuted != info.mute) {
            sinkVolume = volume;
            sinkMuted = info.mute;
            Q_EMIT updateVolume(sinkVolume,sinkMuted);
        }
    }

    if (info.ports) {
        for (pa_sink_port_info ** sinkPort = info.ports; *sinkPort != nullptr; ++sinkPort) {
            temp.insertMulti(info.name,(*sinkPort)->name);
        }
        sinkPortMap.insert(info.card,temp);

        qDebug() << "updateSink" << info.volume.channels << info.active_port->description << info.active_port->name << sinkVolume <<"balance：" <<balance << "defauleSinkName:" <<defaultSinkName.data() << "sinkport" << sinkPortName;

        const char *icon;
    //    std::map<uint32_t, UkmediaCard*>::iterator cw;
        std::set<pa_sink_port_info,sink_port_prio_compare> port_priorities;


        port_priorities.clear();
        for (uint32_t i=0; i<info.n_ports; ++i) {
            port_priorities.insert(*info.ports[i]);
        }

        w->ports.clear();
    }
    if (is_new)
        updateDeviceVisibility();
    return is_new;
}

/*
 * stream suspend callback
 */
static void suspended_callback(pa_stream *s, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (pa_stream_is_suspended(s))
        w->updateVolumeMeter(pa_stream_get_device_index(s), PA_INVALID_INDEX, -1);
}

void UkmediaVolumeControl::readCallback(pa_stream *s, size_t length, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    const void *data;
    double v;
    int index;

    index = pa_stream_get_device_index(s);
    QString str = pa_stream_get_device_name(s);
    QString sss = w->defaultSourceName;
    if (index == w->sourceIndex && strcmp(str.toLatin1().data(),sss.toLatin1().data()) == 0) {
        if (pa_stream_peek(s, &data, &length) < 0) {
            w->showError(UkmediaVolumeControl::tr("Failed to read data from stream").toUtf8().constData());
            return;
        }
    }
    else {
        return;
    }

    if (!data) {
        /* nullptr data means either a hole or empty buffer.
         * Only drop the stream when there is a hole (length > 0) */
        if (length)
            pa_stream_drop(s);
        return;
    }

    assert(length > 0);
    assert(length % sizeof(float) == 0);

    v = ((const float*) data)[length / sizeof(float) -1];

    pa_stream_drop(s);

    if (v < 0)
        v = 0;
    if (v > 1)
        v = 1;

    if (index == w->sourceIndex && strcmp(str.toLatin1().data(),sss.toLatin1().data()) == 0 && !strstr(str.toLatin1().data(),"monitor")){
        w->updateVolumeMeter(index, pa_stream_get_monitor_stream(s), v);
    }
}

pa_stream* UkmediaVolumeControl::createMonitorStreamForSource(uint32_t source_idx, uint32_t stream_idx = -1, bool suspend = false) {
    pa_stream *s;
    char t[16];
    pa_buffer_attr attr;
    pa_sample_spec ss;
    pa_stream_flags_t flags;

    ss.channels = 1;
    ss.format = PA_SAMPLE_FLOAT32;
    ss.rate = 25;

    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = (uint32_t) -1;

    snprintf(t, sizeof(t), "%u", source_idx);
    m_pPaContext = getContext();
    if (!(s = pa_stream_new(getContext(), tr("Peak detect").toUtf8().constData(), &ss, nullptr))) {
        showError(tr("Failed to create monitoring stream").toUtf8().constData());
        return nullptr;
    }

    if (stream_idx != (uint32_t) -1)
        pa_stream_set_monitor_stream(s, stream_idx);

    pa_stream_set_read_callback(s, readCallback, this);
    pa_stream_set_suspended_callback(s, suspended_callback, this);

    flags = (pa_stream_flags_t) (PA_STREAM_DONT_MOVE | PA_STREAM_PEAK_DETECT | PA_STREAM_ADJUST_LATENCY |
                                 (suspend ? PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND : PA_STREAM_NOFLAGS) /*|
                                 (!showVolumeMetersCheckButton->isChecked() ? PA_STREAM_START_CORKED : PA_STREAM_NOFLAGS)*/);

    if (pa_stream_connect_record(s, t, &attr, flags) < 0) {
        showError(tr("Failed to connect monitoring stream").toUtf8().constData());
        pa_stream_unref(s);
        return nullptr;
    }
    return s;
}


void UkmediaVolumeControl::updateSource(const pa_source_info &info) {
    bool is_new = false;
    int volume;
    if (info.volume.channels >= 2)
        volume = MAX(info.volume.values[0],info.volume.values[1]);
    else
        volume = info.volume.values[0];

    //默认的输出音量
    if (info.name && strcmp(defaultSourceName.data(),info.name) == 0) {
        if (info.active_port) {
            if (strcmp(sourcePortName.toLatin1().data(),info.active_port->name) != 0) {
                sourcePortName = info.active_port->name;
                QTimer::singleShot(50, this, SLOT(timeoutSlot()));
            }
	    else
            sourcePortName = info.active_port->name;

        }
        sourceIndex = info.index;
        defaultInputCard = info.card;
        if (sourceVolume != volume || sourceMuted != info.mute) {
            sourceVolume = volume;
            sourceMuted = info.mute;
            Q_EMIT updateSourceVolume(sourceVolume,sourceMuted);
        }
    }

    if (info.index == sourceIndex && !strstr(info.name,".monitor") && !sourceOutputVector.contains(info.index) && pa_context_get_server_protocol_version(getContext()) >= 13) {
        sourceOutputVector.append(info.index);
        sourceFlags = info.flags;
        qDebug() << "createMonitorStreamForSource" <<info.index <<info.name <<defaultSourceName.data();
        if(info.name ==defaultSourceName)
            peak = createMonitorStreamForSource(info.index, -1, !!(info.flags & PA_SOURCE_NETWORK));
    }

    QMap<QString,QString>temp;

    if(info.ports) {
        for (pa_source_port_info ** sourcePort = info.ports; *sourcePort != nullptr; ++sourcePort) {
            temp.insertMulti(info.name,(*sourcePort)->name);
        }
        sourcePortMap.insert(info.card,temp);
    }
    qDebug() << "update source";

    if (is_new)
        updateDeviceVisibility();
}


void UkmediaVolumeControl::setIconFromProplist(QLabel *icon, pa_proplist *l, const char *def) {
    const char *t;

    if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_WINDOW_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ROLE))) {

        if (strcmp(t, "video") == 0 ||
            strcmp(t, "phone") == 0)
            goto finish;

        if (strcmp(t, "music") == 0) {
            t = "audio";
            goto finish;
        }

        if (strcmp(t, "game") == 0) {
            t = "applications-games";
            goto finish;
        }

        if (strcmp(t, "event") == 0) {
            t = "dialog-information";
            goto finish;
        }
    }

    t = def;

finish:

    setIconByName(icon, t);
}


void UkmediaVolumeControl::updateSinkInput(const pa_sink_input_info &info) {
    const char *t;
    if ((t = pa_proplist_gets(info.proplist, "module-stream-restore.id"))) {
        if (t && strcmp(t, "sink-input-by-media-role:event") == 0) {
            g_debug("%s", tr("Ignoring sink-input due to it being designated as an event and thus handled by the Event widget").toUtf8().constData());
            return;
        }
    }
    const gchar *description = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_NAME);
    const gchar *appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);

    //没制定应用名称的不加入到应用音量中
    if (description && !strstr(description,"QtPulseAudio")) {
        if (!info.corked) {

            sinkInputMap.insert(description,info.volume.values[0]);
            if (appId && !sinkInputList.contains(description)) {
                sinkInputList.append(description);
                Q_EMIT addSinkInputSignal(description,appId,info.index);
            }
        }
        else {
            Q_EMIT removeSinkInputSignal(description);
            sinkInputList.removeAll(description);
            QMap<QString,int>::iterator it;
            for(it = sinkInputMap.begin();it!=sinkInputMap.end();)
            {
                if(it.key() == description)
                {
                    sinkInputMap.erase(it);
                    break;
                }
                ++it;
            }
        }
    }
}

void UkmediaVolumeControl::updateSourceOutput(const pa_source_output_info &info) {
    const char *app;

    if(info.name && strstr(info.name,"Peak detect") && !sourceOutputVector.contains(info.source)) {
        pa_operation* o;
        qDebug() <<"killall source output index====" <<peakDetectIndex;
        if (!(o = pa_context_kill_source_output(getContext(), peakDetectIndex, nullptr, nullptr))) {
            showError(tr("pa_context_set_default_source() failed").toUtf8().constData());
//            return;
        }
        sourceOutputVector.removeAt(0);
    }

    if ((app = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID)))
        if (app && strcmp(app, "org.PulseAudio.pavucontrol") == 0
            || strcmp(app, "org.gnome.VolumeControl") == 0
            || strcmp(app, "org.kde.kmixd") == 0)
            return;

    const gchar *description = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_NAME);
    const gchar *appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);

    //没制定应用名称的不加入到应用音量中
    if (description && !strstr(description,"QtPulseAudio")) {
        if (appId && !info.corked) {
            sourceOutputMap.insert(description,info.volume.values[0]);
            Q_EMIT addSourceOutputSignal(description,appId,info.index);
        }
        else {
            Q_EMIT removeSourceOutputSignal(description);
            QMap<QString,int>::iterator it;
            for(it = sourceOutputMap.begin();it!=sourceOutputMap.end();)
            {
                if(it.key() == description)
                {
                    sourceOutputMap.erase(it);
                    break;
                }
                ++it;
            }
        }
    }

}

void UkmediaVolumeControl::updateClient(const pa_client_info &info) {
    g_free(clientNames[info.index]);
    clientNames[info.index] = g_strdup(info.name);
}

void UkmediaVolumeControl::updateServer(const pa_server_info &info) {
    m_pServerInfo = &info;
    defaultSourceName = info.default_source_name ? info.default_source_name : "";
    defaultSinkName = info.default_sink_name ? info.default_sink_name : "";
    qDebug() << "default_sink" << info.default_sink_name << "default_source" << info.default_source_name;

}

void UkmediaVolumeControl::updateVolumeMeter(uint32_t index, uint32_t sinkInputIdx, double v)
{
    Q_UNUSED(index);
    Q_UNUSED(sinkInputIdx);
    if (lastPeak >= DECAY_STEP)
        if (v < lastPeak - DECAY_STEP)
            v = lastPeak - DECAY_STEP;

    lastPeak = v;
    Q_EMIT peakChangedSignal(v);
}

static guint idleSource = 0;

gboolean idleCb(gpointer data) {
    ((UkmediaVolumeControl*) data)->reallyUpdateDeviceVisibility();
    idleSource = 0;
    return FALSE;
}

void UkmediaVolumeControl::setConnectionState(gboolean connected) {
    if (m_connected != connected) {
        m_connected = connected;
        if (m_connected) {
//            connectingLabel->hide();
//            notebook->show();
        } else {
//            notebook->hide();
//            connectingLabel->show();
        }
    }
}

void UkmediaVolumeControl::updateDeviceVisibility() {

    if (idleSource)
        return;

    idleSource = g_idle_add(idleCb, this);
}

void UkmediaVolumeControl::reallyUpdateDeviceVisibility() {
    bool is_empty = true;

//    for (auto & sinkInputWidget : sinkInputWidgets) {
//        SinkInputWidget* w = sinkInputWidget.second;

//        if (sinkWidgets.size() > 1) {
//            w->directionLabel->show();
//            w->deviceButton->show();
//        } else {
//            w->directionLabel->hide();
//            w->deviceButton->hide();
//        }

//        if (showSinkInputType == SINK_INPUT_ALL || w->type == showSinkInputType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (eventRoleWidget)
//        is_empty = false;

//    if (is_empty)
//        noStreamsLabel->show();
//    else
//        noStreamsLabel->hide();

    is_empty = true;

//    for (auto & sourceOutputWidget : sourceOutputWidgets) {
//        SourceOutputWidget* w = sourceOutputWidget.second;

//        if (sourceWidgets.size() > 1) {
//            w->directionLabel->show();
//            w->deviceButton->show();
//        } else {
//            w->directionLabel->hide();
//            w->deviceButton->hide();
//        }

//        if (showSourceOutputType == SOURCE_OUTPUT_ALL || w->type == showSourceOutputType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noRecsLabel->show();
//    else
//        noRecsLabel->hide();

//    is_empty = true;

//    for (auto & sinkWidget : sinkWidgets) {
//        SinkWidget* w = sinkWidget.second;

//        if (showSinkType == SINK_ALL || w->type == showSinkType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noSinksLabel->show();
//    else
//        noSinksLabel->hide();

//    is_empty = true;

//    for (auto & cardWidget : cardWidgets) {
//        CardWidget* w = cardWidget.second;

//        w->show();
//        is_empty = false;
//    }

//    if (is_empty)
//        noCardsLabel->show();
//    else
//        noCardsLabel->hide();

//    is_empty = true;

//    for (auto & sourceWidget : sourceWidgets) {
//        SourceWidget* w = sourceWidget.second;

//        if (showSourceType == SOURCE_ALL ||
//            w->type == showSourceType ||
//            (showSourceType == SOURCE_NO_MONITOR && w->type != SOURCE_MONITOR)) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noSourcesLabel->show();
//    else
//        noSourcesLabel->hide();

//    /* Hmm, if I don't call hide()/show() here some widgets will never
//     * get their proper space allocated */
//    sinksVBox->hide();
//    sinksVBox->show();
//    sourcesVBox->hide();
//    sourcesVBox->show();
//    streamsVBox->hide();
//    streamsVBox->show();
//    recsVBox->hide();
//    recsVBox->show();
//    cardsVBox->hide();
//    cardsVBox->show();
}

void UkmediaVolumeControl::removeCard(uint32_t index) {
//    if (!cardWidgets.count(index))
//        return;

//    delete cardWidgets[index];
//    cardWidgets.erase(index);
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeSink(uint32_t index)
{
    QMap<int,QString>::iterator it;
    for (it=sinkMap.begin();it!=sinkMap.end();)  {
        if (it.key() == index) {
            qDebug() << "removeSink" << index;
            sinkMap.erase(it);
            break;
        }
        ++it;
    }

    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeSource(uint32_t index)
{
    QMap<int,QString>::iterator it;
    for (it=sourceMap.begin();it!=sourceMap.end();)  {
        if (it.key() == index) {
            qDebug() << "removeSource" << index;
            sourceMap.erase(it);
            break;
        }
        ++it;
    }
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeSinkInput(uint32_t index) {
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeSourceOutput(uint32_t index) {
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeClient(uint32_t index) {
    g_free(clientNames[index]);
    clientNames.erase(index);
}


void UkmediaVolumeControl::setConnectingMessage(const char *string) {
    QByteArray markup = "<i>";
    if (!string)
        markup += tr("Establishing connection to PulseAudio. Please wait...").toUtf8().constData();
    else
        markup += string;
    markup += "</i>";
//    connectingLabel->setText(QString::fromUtf8(markup));
}

void UkmediaVolumeControl::showError(const char *txt) {
    char buf[256];

    snprintf(buf, sizeof(buf), "%s: %s", txt, pa_strerror(pa_context_errno(context)));

    qDebug() <<QString::fromUtf8(buf);
//    QMessageBox::information(nullptr, QObject::tr("Error"), QString::fromUtf8(buf));
}

void UkmediaVolumeControl::decOutstanding(UkmediaVolumeControl *w) {
//    qDebug() << "decOutstanding---------";
    if (n_outstanding <= 0)
        return;

    if (--n_outstanding <= 0) {
        // w->get_window()->set_cursor();
        w->setConnectionState(true);
    }
}

void UkmediaVolumeControl::cardCb(pa_context *c, const pa_card_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Card callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    w->cardMap.insert(i->index,i->name);
    w->updateCard(w,*i);
}

void UkmediaVolumeControl::sinkIndexCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Card callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        return;
    }
    int volume;
    if (i->volume.channels >= 2)
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        volume = i->volume.values[0];
    w->defaultOutputCard = i->card;
    w->sinkIndex= i->index;

    if(i->active_port)
        w->sinkPortName = i->active_port->name;
    else
        w->sinkPortName = "";

    if(w->sinkVolume != volume || w->sinkMuted){
        w->sinkVolume = volume;
        w->sinkMuted  = i->mute;
        Q_EMIT w->updateVolume(w->sinkVolume,w->sinkMuted);
    }
}

void UkmediaVolumeControl::sourceIndexCb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        return;
    }
    int volume;
    if(i->volume.channels >= 2)
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        volume = i->volume.values[0];

    w->defaultInputCard = i->card;
    w->sourceIndex = i->index;

    if(i->active_port)
        w->sourcePortName = i->active_port->name;
    else if(strcmp(i->name,"noiseReduceSource")!=0)
        w->sourcePortName = "" ;
//    else
//        w->sourcePortName = "" ;

    if(w->sourceVolume != volume || w->sourceMuted != i->mute){
        w->sourceVolume = volume;
        w->sourceMuted  = i->mute;
        Q_EMIT w->updateSourceVolume(w->sourceVolume,w->sourceMuted);
    }
}

void UkmediaVolumeControl::sinkCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    w->m_pDefaultSink = i;
    qDebug() << "SinkCb" <<i->name <<w->m_pDefaultSink->name << i->volume.values[0] ;
    w->sinkMap.insert(i->index,i->name);

    w->updateSink(w,*i);
}

void UkmediaVolumeControl::sourceCb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    qDebug() << "sourceCb" << i->name << i->description << i->volume.values[PA_CHANNELS_MAX];
    w->sourceMap.insert(i->index,i->name);
    w->updateSource(*i);
}

void UkmediaVolumeControl::sinkInputCb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink input callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }

    w->updateSinkInput(*i);
}

void UkmediaVolumeControl::sourceOutputCb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source output callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0)  {

        if (n_outstanding > 0) {
            /* At this point all notebook pages have been populated, so
             * let's open one that isn't empty */
        }

        decOutstanding(w);
        return;
    }
    if (i->name)
        qDebug() << "sourceOutputCb" << i->name << i->source <<eol ;
    if (!w->sourceOutputVector.contains(i->index)) {
//        w->sourceOutputVector.append(i->index);
        w->updateSourceOutput(*i);
        if (strstr(i->name,"Peak detect"))
            w->peakDetectIndex = i->index;
        qDebug() << "update source output" << w->peakDetectIndex <<i->name;
    }
}

void UkmediaVolumeControl::clientCb(pa_context *c, const pa_client_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Client callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
//    qDebug() << "clientCb" << i->name;
    w->updateClient(*i);
}

void UkmediaVolumeControl::serverInfoCb(pa_context *, const pa_server_info *i, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (!i) {
        w->showError(QObject::tr("Server info callback failure").toUtf8().constData());
        return;
    }

    pa_operation *o;
    //默认的输出设备改变时需要获取默认的输出音量
    if(!(o = pa_context_get_sink_info_by_name(w->getContext(),i->default_sink_name,sinkIndexCb,w))) {
        w->showError(tr("pa_context_get_sink_info_by_name() failed").toUtf8().constData());
    }
    if(!(o = pa_context_get_source_info_by_name(w->getContext(),i->default_source_name,sourceIndexCb,w))) {
        w->showError(tr("pa_context_get_source_info_by_name() failed").toUtf8().constData());
    }
    w->updateServer(*i);
    qDebug() << "serverInfoCb" << i->user_name << i->default_sink_name << w->sinkVolume << i->default_source_name;
    QTimer::singleShot(50, w, [=](){
        qDebug() << "deviceChangedSignal" << w->defaultSinkName <<w->defaultSourceName;
        Q_EMIT w->deviceChangedSignal();
    });

    decOutstanding(w);
}

void UkmediaVolumeControl::timeoutSlot()
{
    Q_EMIT deviceChangedSignal();
}

void UkmediaVolumeControl::extStreamRestoreReadCb(
        pa_context *c,
        const pa_ext_stream_restore_info *i,
        int eol,
        void *userdata) {

    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        decOutstanding(w);
        g_debug(QObject::tr("Failed to initialize stream_restore extension: %s").toUtf8().constData(), pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
//    qDebug() << "extStreamRestoreReadCb" << i->name;
//    w->updateRole(*i);
}

void UkmediaVolumeControl::extStreamRestoreSubscribeCb(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    pa_operation *o;

    if (!(o = pa_ext_stream_restore_read(c, extStreamRestoreReadCb, w))) {
        w->showError(QObject::tr("pa_ext_stream_restore_read() failed").toUtf8().constData());
        return;
    }
    qDebug() << "extStreamRestoreSubscribeCb" ;
    pa_operation_unref(o);
}

void UkmediaVolumeControl::extDeviceManagerReadCb(
        pa_context *c,
        const pa_ext_device_manager_info *,
        int eol,
        void *userdata) {

    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        decOutstanding(w);
        g_debug(QObject::tr("Failed to initialize device manager extension: %s").toUtf8().constData(), pa_strerror(pa_context_errno(c)));
        return;
    }

    w->canRenameDevices = true;

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    qDebug() << "extDeviceManagerReadCb";
    /* Do something with a widget when this part is written */
}

void UkmediaVolumeControl::extDeviceManagerSubscribeCb(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    pa_operation *o;

    if (!(o = pa_ext_device_manager_read(c, extDeviceManagerReadCb, w))) {
        w->showError(QObject::tr("pa_ext_device_manager_read() failed").toUtf8().constData());
        return;
    }
    qDebug() << "extDeviceManagerSubscribeCb";
    pa_operation_unref(o);
}

void UkmediaVolumeControl::subscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSink(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sinkCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSource(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_source_info_by_index(c, index, sourceCb, w))) {
                    w->showError(QObject::tr("pa_context_get_source_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSinkInput(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_input_info(c, index, sinkInputCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_input_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSourceOutput(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_source_output_info(c, index, sourceOutputCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_input_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CLIENT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeClient(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_client_info(c, index, clientCb, w))) {
                    w->showError(QObject::tr("pa_context_get_client_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER: {
                pa_operation *o;
                if (!(o = pa_context_get_server_info(c, serverInfoCb, w))) {
                    w->showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CARD:
            //remove card
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                qDebug() << "remove cards------";
                //移除outputPort

                w->removeOutputPortMap(index);
                w->removeInputPortMap(index);
                w->removeCardMap(index);
                w->removeCardProfileMap(index);

                w->removeProfileMap();
                w->removeInputProfile();
                w->removeCard(index);
                Q_EMIT w->updatePortSignal();
            }
            else {
                pa_operation *o;
                if (!(o = pa_context_get_card_info_by_index(c, index, cardCb, w))) {
                    w->showError(QObject::tr("pa_context_get_card_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

    }
}

void UkmediaVolumeControl::contextStateCallback(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    g_assert(c);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:

            break;

        case PA_CONTEXT_READY: {
            pa_operation *o;
            qDebug() << "pa_context_get_state" << "PA_CONTEXT_READY" << pa_context_get_state(c);
            reconnect_timeout = 1;

            /* Create event widget immediately so it's first in the list */

            pa_context_set_subscribe_callback(c, subscribeCb, w);

            if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                           (PA_SUBSCRIPTION_MASK_SINK|
                                            PA_SUBSCRIPTION_MASK_SOURCE|
                                            PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                            PA_SUBSCRIPTION_MASK_CLIENT|
                                            PA_SUBSCRIPTION_MASK_SERVER|
                                            PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr))) {
                w->showError(QObject::tr("pa_context_subscribe() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);

            /* Keep track of the outstanding callbacks for UI tweaks */
            n_outstanding = 0;

            if (!(o = pa_context_get_server_info(c, serverInfoCb, w))) {
                w->showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_client_info_list(c, clientCb, w))) {
                w->showError(QObject::tr("pa_context_client_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_card_info_list(c, cardCb, w))) {
                w->showError(QObject::tr("pa_context_get_card_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_sink_info_list(c, sinkCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_source_info_list(c, sourceCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_sink_input_info_list(c, sinkInputCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_input_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_source_output_info_list(c, sourceOutputCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_output_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;
            Q_EMIT w->paContextReady();
            break;
        }

        case PA_CONTEXT_FAILED:
            w->setConnectionState(false);
            w->updateDeviceVisibility();
            pa_context_unref(w->m_pPaContext);
            w->m_pPaContext = nullptr;

            if (reconnect_timeout > 0) {
                g_debug("%s", QObject::tr("Connection failed, attempting reconnect").toUtf8().constData());
//                g_timeout_add_seconds(reconnect_timeout, connectToPulse, w);
            }
            return;

        case PA_CONTEXT_TERMINATED:
        default:
            return;
    }
}

pa_context* UkmediaVolumeControl::getContext(void) {
    return context;
}

gboolean UkmediaVolumeControl::connectToPulse(gpointer userdata) {
    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    api = pa_glib_mainloop_get_api(m);

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, QObject::tr("Ukui Media Volume Control").toUtf8().constData());
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.PulseAudio.pavucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "PACKAGE_VERSION");
    context = pa_context_new_with_proplist(api, nullptr, proplist);
    g_assert(context);

    pa_proplist_free(proplist);

    pa_context_set_state_callback(getContext(), contextStateCallback, this);
    if (pa_context_connect(getContext(), nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(getContext()) == PA_ERR_INVALID) {
            /*w->setConnectingMessage(QObject::tr("Connection to PulseAudio failed. Automatic retry in 5s\n\n"
                "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties\n"
                "or default-server in client.conf is misconfigured.\n"
                "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.\n"
                "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should\n"
                "run start-pulseaudio-x11 manually.").toUtf8().constData());*/
            qFatal("connect pulseaudio failed")  ;
        }
        else {
//            g_timeout_add_seconds(5,connectToPulse,w);
        }
    }

    return false;
}


/*
 * 根据名称获取sink input音量
 */
int UkmediaVolumeControl::getSinkInputVolume(const gchar *name)
{
    QMap<QString,int>::iterator it;
    int value = 0;
    for(it = sinkInputMap.begin();it!=sinkInputMap.end();)
    {
        if(it.key() == name)
        {
            qDebug() << "getSinkInputVolume" << "name:" <<name << "it.key" <<it.key() << "it.value" <<it.value();
            value = it.value();
            return value;
        }
        ++it;
    }
    return value;
}

/*
 * 根据名称获取source output音量
 */
int UkmediaVolumeControl::getSourceOutputVolume(const gchar *name)
{
    QMap<QString,int>::iterator it;
    int value = 0;
    for(it = sourceOutputMap.begin();it!=sourceOutputMap.end();)
    {
        if(it.key() == name)
        {
            qDebug() << "getSourceOutputVolume" << "name:" <<name << "it.key" <<it.key() << "it.value" <<it.value();
            value = it.value();
            return value;
        }
        ++it;
    }
    return value;
}


void UkmediaVolumeControl::sinkInputCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink input callback failure").toUtf8().constData());
        return;
    }
    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    w->sinkInputMuted = i->mute;
    if (i->volume.channels >= 2)
        w->sinkInputVolume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        w->sinkInputVolume = i->volume.values[0];
    qDebug() << "sinkInputCallback" <<w->sinkInputVolume <<i->name;
}

/*
 * 更新output port map
 */
void UkmediaVolumeControl::updateOutputPortMap()
{
    QMap<int,QString>::iterator at;
    QMap<int,QString>::iterator cardNameMap;
//    if (firstEntry == true) {

//        for(at = outputPortMap.begin();at!=w->outputPortMap.end();)
//        {

//            UkuiListWidgetItem *itemW = new UkuiListWidgetItem(w);

//            QListWidgetItem * item = new QListWidgetItem(w->m_pOutputWidget->m_pOutputListWidget);
//            item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
//            w->m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
//            w->m_pOutputWidget->m_pOutputListWidget->setItemWidget(item, itemW);
//            w->m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
//            QString cardName;
//            for(cardNameMap = w->cardMap.begin();cardNameMap!=w->cardMap.end();)
//            {
//                if (cardNameMap.key() == at.key()) {
//                    cardName = cardNameMap.value();
//                    break;
//                }
//                ++cardNameMap;
//            }

//            itemW->setLabelText(at.value(),cardName);
//            w->m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
//            w->m_pOutputWidget->m_pOutputListWidget->insertItem(i++,item);
//            w->m_pOutputWidget->m_pOutputListWidget->blockSignals(false);

//            ++at;
//        }
//        for(at = w->inputPortLabelMap.begin();at!=w->inputPortLabelMap.end();)
//        {
//            UkuiListWidgetItem *itemW = new UkuiListWidgetItem(w);

//            QListWidgetItem * item = new QListWidgetItem(w->m_pInputWidget->m_pInputListWidget);
//            item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
//            w->m_pInputWidget->m_pInputListWidget->setItemWidget(item, itemW);
//            QString cardName;
//            for(cardNameMap = w->cardMap.begin();cardNameMap!=w->cardMap.end();)
//            {
//                if (cardNameMap.key() == at.key()) {
//                    cardName = cardNameMap.value();
//                    break;
//                }
//                ++cardNameMap;
//            }
//            itemW->setLabelText(at.value(),cardName);
//            w->m_pInputWidget->m_pInputListWidget->blockSignals(true);
//            w->m_pInputWidget->m_pInputListWidget->insertItem(i++,item);
//            w->m_pInputWidget->m_pInputListWidget->blockSignals(false);

//            ++at;
//        }

//    }
//    else {
//        //记录上一次output label
//        for (i=0;i<w->m_pOutputWidget->m_pOutputListWidget->count();i++) {

//            QListWidgetItem *item = w->m_pOutputWidget->m_pOutputListWidget->item(i);
//            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)w->m_pOutputWidget->m_pOutputListWidget->itemWidget(item);
//            int index;
//            for (at=w->cardMap.begin();at!=w->cardMap.end();) {
//                if (wid->deviceLabel->text() == at.value()) {
//                    index = at.key();
//                    break;
//                }
//                ++at;
//            }
//            w->currentOutputPortLabelMap.insertMulti(index,wid->portLabel->text());
//            w->m_pCurrentOutputPortLabelList->append(wid->portLabel->text());
//            w->m_pCurrentOutputCardList->append(wid->deviceLabel->text());
//            //qDebug() << index << "current output item ************" << item->text() <<wid->portLabel->text() << w->m_pOutputPortLabelList->count() ;//<< w->m_pOutputPortLabelList->at(i);
//        }

//        for (i=0;i<w->m_pInputWidget->m_pInputListWidget->count();i++) {

//            QListWidgetItem *item = w->m_pInputWidget->m_pInputListWidget->item(i);
//            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)w->m_pInputWidget->m_pInputListWidget->itemWidget(item);
//            int index;
//            int count;
//            for (at=w->cardMap.begin();at!=w->cardMap.end();) {
//                if (wid->deviceLabel->text() == at.value()) {
//                    index = at.key();
//                    break;
//                }
//                ++at;
//                ++count;
//            }
//            w->currentInputPortLabelMap.insertMulti(index,wid->portLabel->text());
//            w->m_pCurrentInputPortLabelList->append(wid->portLabel->text());
//            w->m_pCurrentInputCardList->append(wid->deviceLabel->text());
//            qDebug() <<"current input port label insert " << index << wid->deviceLabel->text();
//        }
////        w->m_pInputWidget->m_pInputListWidget->blockSignals(true);
////        w->deleteNotAvailableOutputPort();
////        w->addAvailableOutputPort();
////        w->deleteNotAvailableInputPort();
////        w->addAvailableInputPort();
////        w->m_pInputWidget->m_pInputListWidget->blockSignals(false);
//    }
}


/*
 * 移除指定索引的output port
 */
void UkmediaVolumeControl::removeOutputPortMap(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        if (it.key() == index) {
            qDebug() << "removeoutputport" <<it.key() << it.value();
            outputPortMap.erase(it);
            break;
        }
        ++it;
    }
}

/*
 * 移除指定索引的input port
 */
void UkmediaVolumeControl::removeInputPortMap(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    for (it=inputPortMap.begin();it!=inputPortMap.end();) {
        if (it.key() == index) {
            inputPortMap.erase(it);
            break;
        }
        ++it;
    }
}

/*
 * 移除指定索引的card
 */
void UkmediaVolumeControl::removeCardMap(int index)
{
    QMap<int, QString>::iterator it;
    for (it=cardMap.begin();it!=cardMap.end();) {
        if (it.key() == index) {
            cardMap.erase(it);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeCardProfileMap(int index)
{
    QMap<int, QList<QString>>::iterator it;
    QMap<int, QMap<QString,int>>::iterator at;
    for (it=cardProfileMap.begin();it!=cardProfileMap.end();) {
        if (it.key() == index) {
            cardProfileMap.erase(it);
            break;
        }
        ++it;
    }

    for (at=cardProfilePriorityMap.begin();at!=cardProfilePriorityMap.cend();) {
        if (at.key() == index) {
            cardProfilePriorityMap.erase(at);
            break;
        }
        ++at;
    }
}

void UkmediaVolumeControl::removeProfileMap()
{
    QMap<QString,QString>::iterator it;
    for (it=profileNameMap.begin();it!=profileNameMap.end();) {
        qDebug() << "ctf ------------" << profileNameMap.count();
        qDebug() << "removeProfileMap" <<it.key() <<it.value();

        if (!isExitOutputPort(it.value())) {

            it = profileNameMap.erase(it);
            continue;
        }
        ++it;
    }

}

bool UkmediaVolumeControl::isExitOutputPort(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        portMap = it.value();
        for (at=portMap.begin();at!=portMap.end();) {
            if (at.value() == name)
                return true;
            ++at;
        }
        ++it;
    }
    return false;
}

void UkmediaVolumeControl::removeInputProfile()
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> temp;
    for (it=inputPortProfileNameMap.begin();it!=inputPortProfileNameMap.end();) {
        temp = it.value();
        for (at=temp.begin();at!=temp.end();) {

            if (!isExitInputPort(at.value())) {
                it = inputPortProfileNameMap.erase(it);
                return;
            }
            ++at;
        }
        ++it;
    }
}

bool UkmediaVolumeControl::isExitInputPort(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for (it=inputPortMap.begin();it!=inputPortMap.end();) {
        portMap = it.value();
        for (at=portMap.begin();at!=portMap.end();) {
            if (at.value() == name)
                return true;
            ++at;
        }
        ++it;
    }
    return false;
}
