#include "kbdlayoutmanager.h"
#include "ui_kbdlayoutmanager.h"

#include <QDebug>

extern "C" {
#include <libxklavier/xklavier.h>
#include <libmatekbd/matekbd-keyboard-config.h>
}

#define MAXNUM 4

XklEngine * engine;
XklConfigRegistry * config_registry;

static void kbd_set_countries(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout>  *list);

static void kbd_set_languages(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout>  *list);

static void kbd_set_available_countries(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list);

static void kbd_set_available_languages(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list);

QList<Layout> languages;
QList<Layout> countries;
QStringList availablelayoutsList;


KbdLayoutManager::KbdLayoutManager(QStringList ll, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KbdLayoutManager),
    layoutsList(ll)
{
    ui->setupUi(this);

    engine = xkl_engine_get_instance (QX11Info::display());
    config_registry = xkl_config_registry_get_instance (engine);

    xkl_config_registry_load (config_registry, false);

    xkl_config_registry_foreach_country(config_registry,(ConfigItemProcessFunc)kbd_set_countries, NULL);

    xkl_config_registry_foreach_language(config_registry,(ConfigItemProcessFunc)kbd_set_languages, NULL);


    component_init();
    setup_component();

}

KbdLayoutManager::~KbdLayoutManager()
{
    delete ui;
}

void KbdLayoutManager::component_init(){
    ui->countryRadioButton->setChecked(true);

    // init listwidget intalled
    for (QString layout : layoutsList){
        create_listwidgetitem(layout);
//        QString desc = kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data()));
//        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
//        item->setText(desc);
//        item->setData(Qt::UserRole, layout);
//        ui->listWidget->addItem(item);
    }

    // init country comboBox
    for (Layout keylayout : countries){
        ui->countryComboBox->addItem(keylayout.desc, keylayout.name);
    }

    // init language comboBox
    for (Layout keylayout : languages){
        ui->languageComboBox->addItem(keylayout.desc, keylayout.name);
    }

    if (ui->listWidget->count() >= MAXNUM)
        ui->addBtn->setEnabled(false);

    refresh_widget_status();
}

void KbdLayoutManager::setup_component(){
    connect(ui->countryComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(countries_changed_slot(int)));
    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languages_changed_slot(int)));
    connect(this, SIGNAL(rebuild_variant_signals(bool,QString)), this, SLOT(rebuild_variant_slots(bool,QString)));

    connect(ui->buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [=]{refresh_variant_combobox();refresh_widget_status();});

    connect(ui->addBtn, &QPushButton::clicked, [=]{emit add_new_variant_signals(ui->variantComboBox->currentData(Qt::UserRole).toString());});

    connect(this, &KbdLayoutManager::add_new_variant_signals, [=](QString id){create_listwidgetitem(id);});

    refresh_variant_combobox();
}

void KbdLayoutManager::create_listwidgetitem(QString layout){
    if (ui->listWidget->count() < MAXNUM){
        QString desc = kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data()));
        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setText(desc);
        item->setData(Qt::UserRole, layout);
        ui->listWidget->addItem(item);
    }

    if (ui->listWidget->count() >= MAXNUM)
        ui->addBtn->setEnabled(false);
    else
        ui->addBtn->setEnabled(true);
}

void KbdLayoutManager::refresh_variant_combobox(){

    if (ui->countryRadioButton->isChecked()){
        QString id = ui->countryComboBox->currentData(Qt::UserRole).toString();
        emit rebuild_variant_signals(true, id);
    }
    else{
        QString id = ui->languageComboBox->currentData(Qt::UserRole).toString();
        emit rebuild_variant_signals(false, id);
    }
}

void KbdLayoutManager::refresh_widget_status(){
    ui->countryWidget->setEnabled(ui->countryRadioButton->isChecked());
    ui->languageWidget->setEnabled(ui->languageRadioButton->isChecked());
}

void KbdLayoutManager::rebuild_variant_combobox(){
    ui->variantComboBox->clear();
    for (QString name : availablelayoutsList){
       QString desc = kbd_get_description_by_id(const_cast<const char *>(name.toLatin1().data()));
       ui->variantComboBox->addItem(desc, name);
    }
}

void KbdLayoutManager::kbd_trigger_available_countries(char *countryid){
    xkl_config_registry_foreach_country_variant (config_registry, countryid, (TwoConfigItemsProcessFunc)kbd_set_available_countries, NULL);
}

void KbdLayoutManager::kbd_trigger_available_languages(char *languageid){
    xkl_config_registry_foreach_language_variant (config_registry, languageid, (TwoConfigItemsProcessFunc)kbd_set_available_languages, NULL);
}

QString KbdLayoutManager::kbd_get_description_by_id(const char *visible){
    char *l, *sl, *v, *sv;
    if (matekbd_keyboard_config_get_descriptions(config_registry, visible, &sl, &l, &sv, &v))
        visible = matekbd_keyboard_config_format_full_layout (l, v);
    return QString(const_cast<char *>(visible));
}

void KbdLayoutManager::countries_changed_slot(int index){
    QString id = ui->countryComboBox->currentData().toString();
    emit rebuild_variant_signals(true, id);
}

void KbdLayoutManager::languages_changed_slot(int index){
    QString id = ui->languageComboBox->currentData().toString();
    emit rebuild_variant_signals(false, id);
}

void KbdLayoutManager::rebuild_variant_slots(bool type, QString id){
    availablelayoutsList.clear();
    char * iid = id.toLatin1().data();
    if (type)
        kbd_trigger_available_countries(iid);
    else
        kbd_trigger_available_languages(iid);
    rebuild_variant_combobox();
}

static void kbd_set_countries(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout> *list){
    Layout item;
    item.desc = config_item->description;
    item.name = config_item->name;

//    qDebug()<<"countries" << "desc = "<<item.desc<<"name = "<<item.name ;

//    list->append(item);
    countries.append(item);
}

static void kbd_set_languages(XklConfigRegistry *config_registry, XklConfigItem *config_item, QList<Layout> *list){
    Layout item;
    item.desc = config_item->description;
    item.name = config_item->name;
//     qDebug()<<"languages" << "desc = "<<item.desc<<"name = "<<item.name;
//    list->append(item);
    languages.append(item);
}

static void kbd_set_available_countries(XklConfigRegistry *config_registry, XklConfigItem * parent_config_item, XklConfigItem *config_item, QList<Layout>  *list){
    const gchar *xkb_id = config_item ? matekbd_keyboard_config_merge_items (parent_config_item->name, config_item->name) : parent_config_item->name;
    availablelayoutsList.append(QString(const_cast<char *>(xkb_id)));
}

static void kbd_set_available_languages(XklConfigRegistry *config_registry, XklConfigItem *parent_config_item, XklConfigItem *config_item, QList<Layout> *list){
    kbd_set_available_countries(config_registry, parent_config_item, config_item, NULL);
}
