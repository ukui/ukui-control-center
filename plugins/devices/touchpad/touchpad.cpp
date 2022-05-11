/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "touchpad.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

Touchpad::Touchpad() : mFirstLoad(true)
{
    pluginName = tr("Touchpad");
    pluginType = DEVICES;
}

Touchpad::~Touchpad()
{
}

QString Touchpad::plugini18nName()
{
    return pluginName;
}

int Touchpad::pluginTypes()
{
    return pluginType;
}

QWidget *Touchpad::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new TouchpadUI;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    }

    return pluginWidget;
}

const QString Touchpad::name() const
{
    return QStringLiteral("Touchpad");
}

bool Touchpad::isShowOnHomePage() const
{
    return true;
}

QIcon Touchpad::icon() const
{
    return QIcon::fromTheme("input-touchpad-symbolic");
}

bool Touchpad::isEnable() const
{
    bool isFound = findSynaptics();
    return isFound;
}

// 判断是否检测到触摸板设备
bool Touchpad::findSynaptics() const
{
    XDeviceInfo *device_info;
    int n_devices;
    bool retval;

    if (_supportsXinputDevices() == false) {
        return true;
    }

    device_info = XListInputDevices (QX11Info::display(), &n_devices);
    if (device_info == nullptr) {
        return false;
    }

    retval = false;
    for (int i = 0; i < n_devices; i++) {
        XDevice *device;

        device = _deviceIsTouchpad (&device_info[i]);
        if (device != nullptr) {
            retval = true;
            break;
        }
    }
    if (device_info != nullptr) {
        XFreeDeviceList (device_info);
    }

    return retval;
}

bool Touchpad::_supportsXinputDevices() const
{
    int op_code, event, error;

    return XQueryExtension (QX11Info::display(),
                            "XInputExtension",
                            &op_code,
                            &event,
                            &error);
}

XDevice* Touchpad::_deviceIsTouchpad (XDeviceInfo *deviceinfo) const
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true)) {
        return nullptr;
    }

    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == nullptr) {
        qDebug()<<"device== null";
        return nullptr;
    }

    if (_deviceHasProperty(device, "libinput Tapping Enabled") ||
            _deviceHasProperty(device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return nullptr;
}

bool Touchpad::_deviceHasProperty(XDevice *device, const char *property_name) const
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (QX11Info::display(), property_name, True);
    if (!prop) {
        return false;
    }

    if ((XGetDeviceProperty (QX11Info::display(), device, prop, 0, 1, False,
                             XA_INTEGER, &realtype, &realformat, &nitems,
                             &bytes_after, &data) == Success) && (realtype != None))
    {
        XFree (data);
        return true;
    }
    return false;
}

