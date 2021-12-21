/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N com.kylin.RemoteDesktop.xml -p krd.h:krd.cpp
 *
 * qdbusxml2cpp is Copyright (C) 2020 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "krd.h"

/*
 * Implementation of interface class ComKylinRemoteDesktopInterface
 */

ComKylinRemoteDesktopInterface::ComKylinRemoteDesktopInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qDBusRegisterMetaType<ClientInfo>();
}

ComKylinRemoteDesktopInterface::~ComKylinRemoteDesktopInterface()
{
}
