#ifndef XMLHANDLE_H
#define XMLHANDLE_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <QString>
#include <QMap>

#define WALLPAPER "/usr/share/ukui-background-properties/"

class XmlHandle{

public:
    XmlHandle();
    ~XmlHandle();

    void init();
    QStringList getxmlfiles(QString path);
    QMap<QString, QMap<QString, QString> > xmlreader(QString filename);
    void parsewallpaper(QXmlStreamReader &reader);
    void xmlwriter(QString targetname, QMap<QString, QMap<QString, QString>> wallpaperinfosMap);

//    QMap<QString, QString> wpMap;
//    QMap<QString, QString> headMap;

private:
    QDir xmlDir;
    void _xmlwriter(QString targetname);

    QMap<QString, QMap<QString, QString>> wallpapersMap;

};

#endif // XMLHANDLE_H
