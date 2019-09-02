#ifndef KBDLAYOUTMANAGER_H
#define KBDLAYOUTMANAGER_H

#include <QWidget>
#include <QX11Info>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif


typedef struct _Layout Layout;

struct _Layout{
    QString desc;
    QString name;
};

namespace Ui {
class KbdLayoutManager;
}

class KbdLayoutManager : public QWidget
{
    Q_OBJECT

public:
    explicit KbdLayoutManager(QStringList ll, QWidget *parent = 0);
    ~KbdLayoutManager();

    QString kbd_get_description_by_id(const char *visible);

    void kbd_trigger_available_countries(char * countryid);
    void kbd_trigger_available_languages(char * languageid);

    void component_init();
    void setup_component();
    void create_listwidgetitem(QString layout);
    void refresh_variant_combobox();
    void refresh_widget_status();
    void rebuild_variant_combobox();

private:
    Ui::KbdLayoutManager *ui;
    QStringList layoutsList;

Q_SIGNALS:
    void rebuild_variant_signals(bool type, QString id);
    void add_new_variant_signals(QString id);

private slots:
    void countries_changed_slot(int index);
    void languages_changed_slot(int index);
    void rebuild_variant_slots(bool type, QString id);
};

#endif // KBDLAYOUTMANAGER_H
