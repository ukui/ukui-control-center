#include <QLibrary>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>

#include <QX11Info>
extern "C" {
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
}
bool isExistTouchScreen(){

    int  ndevices = 0;
    bool retval=false;

    Display *dpy = XOpenDisplay(NULL);
    XIDeviceInfo *info = XIQueryDevice(dpy, XIAllDevices, &ndevices);

    for (int i = 0; i < ndevices; i++)
    {
        XIDeviceInfo* dev = &info[i];
        // 判断当前设备是不是触摸屏
        if(dev->use != XISlavePointer) continue;
        if(!dev->enabled) continue;
        for (int j = 0; j < dev->num_classes; j++)
        {
            if (dev->classes[j]->type == XITouchClass)
            {
                retval = true;
            }
        }
    }

    XIFreeDeviceInfo(info);
    XCloseDisplay(dpy);

    return retval;
}



bool _supportsXinputDevices()
{
    int op_code, event, error;

    return XQueryExtension (QX11Info::display(),
                            "XInputExtension",
                            &op_code,
                            &event,
                            &error);
}

bool _deviceHasProperty(XDevice *device, const char *property_name)
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

XDevice* _deviceIsTouchpad (XDeviceInfo *deviceinfo)
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true)) {
        return nullptr;
    }

    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == nullptr) {
//        qDebug()<<"device== null";
        return nullptr;
    }

    if (_deviceHasProperty(device, "libinput Tapping Enabled") ||
            _deviceHasProperty(device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return nullptr;
}


bool isfindSynaptics()
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
