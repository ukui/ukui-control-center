#ifndef DATADEFINED_H
#define DATADEFINED_H

#include <QPixmap>
#include <QMap>

#define SYSTEMPOS 2
#define ALLPOS 1
#define LOCALPOS 0

#define APP_KEY_FILE_DESKTOP_KEY_AUTOSTART_ENABLE "X-UKUI-Autostart-enabled"

typedef struct _AutoApp{

    QString bname;
    QString path;

    bool enable;
    bool no_display;
    bool shown;
    bool hidden;

    QString name;
    QString comment;
    QPixmap pixmap;
    QString exec;
    QString description;

    int xdg_position;
}AutoApp;

#endif // DATADEFINED_H
