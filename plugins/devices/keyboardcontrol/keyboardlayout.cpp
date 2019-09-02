#include "keyboardlayout.h"

#include <QDebug>

extern "C" {
#include <libxklavier/xklavier.h>
#include <libmatekbd/matekbd-keyboard-config.h>
}

XklEngine * engine;
XklConfigRegistry * config_registry;

static void KeyboardXkb_get_countries(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout>  *list);

static void KeyboardXkb_get_languages(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout>  *list);

static void KeyboardXkb_get_available_countries(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list);

static void KeyboardXkb_get_available_languages(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list);


QList<Layout> languages;
QList<Layout> countries;

KeyboardLayout::KeyboardLayout()
{

    data_init();

    char * id = QString("ee").toLatin1().data();
    QString desc = kbd_get_description_by_id(id);

}

KeyboardLayout::~KeyboardLayout()
{

}

void KeyboardLayout::data_init(){
    engine = xkl_engine_get_instance (QX11Info::display());
    config_registry = xkl_config_registry_get_instance (engine);

    xkl_config_registry_load (config_registry, false);

    xkl_config_registry_foreach_country(config_registry,(ConfigItemProcessFunc)KeyboardXkb_get_countries, NULL);

    xkl_config_registry_foreach_language(config_registry,(ConfigItemProcessFunc)KeyboardXkb_get_languages, NULL);

//    char * country_id = QString("EE").toLatin1().data();
//    xkl_config_registry_foreach_country_variant (config_registry, country_id, (TwoConfigItemsProcessFunc)KeyboardXkb_get_available_countries, NULL);

//    char * language_id = QString("est").toLatin1().data();
//    xkl_config_registry_foreach_language_variant (config_registry, language_id, (TwoConfigItemsProcessFunc)KeyboardXkb_get_available_languages, NULL);
}

QString KeyboardLayout::kbd_get_description_by_id(const char *visible){
    char *l, *sl, *v, *sv;
    if (matekbd_keyboard_config_get_descriptions(config_registry, visible, &sl, &l, &sv, &v))
        visible = matekbd_keyboard_config_format_full_layout (l, v);
    return QString(const_cast<char *>(visible));
}

static void KeyboardXkb_get_countries(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout> *list){
    Layout item;
    item.desc = config_item->description;
    item.name = config_item->name;

    qDebug()<<"countries" << "desc = "<<item.desc<<"name = "<<item.name ;


//    list->append(item);
    countries.append(item);
}

static void KeyboardXkb_get_languages(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout> *list){
    Layout item;
    item.desc = config_item->description;
    item.name = config_item->name;
     qDebug()<<"languages" << "desc = "<<item.desc<<"name = "<<item.name;
//    list->append(item);
    languages.append(item);
}

static void KeyboardXkb_get_available_countries(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list){
    const gchar *xkb_id = config_item ? matekbd_keyboard_config_merge_items (parent_config_item->name, config_item->name) : parent_config_item->name;
    g_warning("------------->%s", xkb_id);
}

static void KeyboardXkb_get_available_languages(XklConfigRegistry *config_registry, XklConfigItem *parent_config_item, XklConfigItem *config_item, QList<Layout> *list){
    KeyboardXkb_get_available_countries(config_registry, parent_config_item, config_item, NULL);
}
