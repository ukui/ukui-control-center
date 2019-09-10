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
