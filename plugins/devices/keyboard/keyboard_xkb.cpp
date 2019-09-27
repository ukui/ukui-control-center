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
#include "keyboard_xkb.h"

QList<Item>languages;
QList<Item>countries;


KeyboardXkb::KeyboardXkb()
{
    engine = xkl_engine_get_instance (QX11Info::display());
    config_registry = xkl_config_registry_get_instance (engine);
	
    xkl_config_registry_load (config_registry, false);
    SetLanguages();
    SetCountries();

   // printf("count - %d\n",countries.count());


}

void KeyboardXkb::SetCountries()
{

    xkl_config_registry_foreach_country(config_registry,(ConfigItemProcessFunc)KeyboardXkb_get_countries, NULL);

}

void KeyboardXkb::SetLanguages()
{
    xkl_config_registry_foreach_language(config_registry,(ConfigItemProcessFunc)KeyboardXkb_get_languages, NULL);

}



QList<Item> KeyboardXkb::GetCountries()
{
  //  printf("countries count  = %d\n",countries.count());
    return countries;
}

QList<Item> KeyboardXkb::GetLanguages()
{
  //  printf("countries count  = %d\n",countries.count());
    return languages;
}

static void KeyboardXkb_get_countries(XklConfigRegistry *config_registry,
                          XklConfigItem *config_item,
                          QList<Item> *list)
{
    Item item;
    item.desc = config_item->description;
    item.name = config_item->name;
  //  qDebug()<<"desc = "<<item.desc<<"name = "<<item.name ;


   // list->append(item);
    countries.append(item);

}

static void KeyboardXkb_get_languages(XklConfigRegistry *config_registry,
                          XklConfigItem *config_item,
                          QList<Item> *list)
{
    Item item;
    item.desc = config_item->description;
    item.name = config_item->name;
   //  qDebug()<<"desc = "<<item.desc<<"name = "<<item.name;
    //list->append(item);
    languages.append(item);
}

