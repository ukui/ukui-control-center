#include "usbthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <regex>
#include <iostream>
#include <QDebug>
#include <QThread>

#define UEVENT_BUFFER_SIZE 2048

UsbThread::UsbThread()
{

}

UsbThread::~UsbThread()
{

}

void UsbThread::run()
{
    int hotplug_sock = init_sock();
    while(1) {
        char buf[UEVENT_BUFFER_SIZE*2] = {0};
        // recv 会等待usb信号连入
        recv(hotplug_sock, &buf, sizeof(buf), 0);
        usbDeviceIdentify(QString(buf));
    }
}

void UsbThread::usbDeviceIdentify(const QString &str)
{
    if (  str.indexOf("bind") == 0
          &&  str.contains("pci")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(":")
          && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(".")) {
            emit keychangedsignal();
    }
    // unbind@/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1
    if (  str.contains("unbind")
      &&  str.contains("pci")
      && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(":")
      && !str.right( str.size() -1 - str.lastIndexOf('/') ).contains(".")) {
        emit keychangedsignal();
    }
    return;
}

int UsbThread::init_sock()
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}
