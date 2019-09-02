#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H

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

class KeyboardLayout : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardLayout();
    ~KeyboardLayout();

    void data_init();

    QString kbd_get_description_by_id(const char *visible);

};

#endif // KEYBOARDLAYOUT_H
