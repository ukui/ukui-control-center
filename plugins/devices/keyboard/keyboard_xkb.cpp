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

