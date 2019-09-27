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
#ifndef KEYBOARD_XKB_H
#define KEYBOARD_XKB_H

#include <QString>
#include <QStringList>
#include <QPointer>
#include <QX11Info>
#include <QDebug>
#include <libxklavier/xklavier.h>
//#include <libmatekbd/matekbd-desktop-config.h>
//#include <libmatekbd/matekbd-keyboard-config.h>


class Item{
	public:
		QString desc;
		QString name;
};

class KeyboardXkb
{
public:
	KeyboardXkb();
	~KeyboardXkb();
	
    QList<Item> GetCountries();
    QList<Item> GetLanguages();
    int getcount();
	void SetCountries();
    void SetLanguages();

	void AddToCountries();
	void AddToLanguages();

	XklEngine *engine;
	XklConfigRegistry *config_registry;

	
private:

};

static void KeyboardXkb_get_countries(XklConfigRegistry *config_registry,
                          XklConfigItem *config_item,
                          QList<Item>  *list);

static void KeyboardXkb_get_languages(XklConfigRegistry *config_registry,
                          XklConfigItem *config_item,
                           QList<Item>  *list);

#endif
