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
#include "xmlhandle.h"

#include <QDebug>

XmlHandle::XmlHandle()
{
    init();
}

XmlHandle::~XmlHandle()
{
}

void XmlHandle::init(){
    QString localconf;
    localconf = QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("wallpaper.xml");
    QFile file(localconf);
    if (!file.exists()){
        QStringList files = getxmlfiles(WALLPAPER);
        for (int num = 0; num < files.length(); num++){
            xmlreader(files[num]);
        }
        _xmlwriter(localconf);
    }
}

QStringList XmlHandle::getxmlfiles(QString path){
    xmlDir = QDir(path);
    QStringList xmlfilesStringList;

    foreach (QString filename, xmlDir.entryList(QDir::Files)) {
        if (filename.endsWith(".xml"))
            xmlfilesStringList.append(QString("%1/%2").arg(WALLPAPER).arg(filename));
    }
    return xmlfilesStringList;
}

QMap<QString, QMap<QString, QString> > XmlHandle::xmlreader(QString filename){
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        QMap<QString, QMap<QString, QString> > tmpMap;
        qDebug() << "Error Open XML file: " << file.errorString();
        return tmpMap;
    }

    //旧清理数据
//    wpMap.clear();
//    headMap.clear();
    wallpapersMap.clear();

    QMap<QString, QString> headMap;

    QXmlStreamReader reader;
    reader.setDevice(&file);

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType nType = reader.readNext();
        switch (nType) {
        case QXmlStreamReader::StartDocument: {
            QString versionStr = reader.documentVersion().toString();
            QString encodingStr = reader.documentEncoding().toString();
            headMap.insert("version", versionStr);
            headMap.insert("encoding", encodingStr);
//            bool aloneBool = reader.isStandaloneDocument();
//            qDebug() << QString::fromLocal8Bit("Version: %1 encoding: %2 standalone: %3").arg(versionStr).arg(encodingStr).arg(aloneBool);
            break;
        }
        case QXmlStreamReader::DTD: {
            QString dtdStr = reader.text().toString();
            QString dtdnameStr = reader.dtdName().toString();
            QString dtdsystemidStr = reader.dtdSystemId().toString();
            headMap.insert("doctype", dtdnameStr);
            headMap.insert("system", dtdsystemidStr);
//            qDebug() << QString::fromLocal8Bit("DTD:%1; DTD name:%2; systemid:%3").arg(dtdStr).arg(dtdnameStr).arg(dtdsystemidStr);
            break;
        }
        case QXmlStreamReader::Comment: {
            QString commentStr = reader.text().toString();
            break;
        }
        case QXmlStreamReader::StartElement: {
            QString elementnameStr = reader.name().toString();
            if (elementnameStr == "wallpapers"){ //根元素
                parsewallpaper(reader);
            }
            break;
        }
        case QXmlStreamReader::EndDocument: {
            break;
        }
        default:
            break;
        }
    }
    wallpapersMap.insert("head", headMap);

    if (reader.hasError()){
        qDebug() << QString::fromLocal8Bit("msg: %1; line: %2; column: %3; char shift: %4").arg(reader.errorString()).arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.characterOffset());
    }
    file.close();

    return wallpapersMap;
}

void XmlHandle::parsewallpaper(QXmlStreamReader &reader){
    QMap<QString, QString> wpMap;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()){
            QString elementnameStr = reader.name().toString();
            if (elementnameStr == "wallpaper"){
                wpMap.clear();
//                qDebug() << QString::fromLocal8Bit("******开始元素<wallpaper>***");
                QXmlStreamAttributes wp_attributes = reader.attributes();
                if (wp_attributes.hasAttribute("deleted")){
                    QString deletedStr = wp_attributes.value("deleted").toString();
                    wpMap.insert("deleted", deletedStr);
                }
            }
            else if (elementnameStr == "name"){
                QXmlStreamAttributes name_attributes = reader.attributes();
                if (name_attributes.hasAttribute("xml:lang")){
                    QString langStr = name_attributes.value("xml:lang").toString();
//                    qDebug() << QString::fromLocal8Bit("name zh_CN: %1").arg(reader.readElementText());
                    wpMap.insert("name.zh_CN", reader.readElementText());
                }
                else
                    wpMap.insert("name", reader.readElementText());
            }
            else
                wpMap.insert(elementnameStr, reader.readElementText());
        }
        else if (reader.isEndElement()){
            QString elementnameStr = reader.name().toString();
            if (elementnameStr == "wallpaper"){
//                QMap<QString, QString> tmpMap;
//                QMap<QString, QString>::iterator it=wpMap.begin();
//                for (;it!=wpMap.end(); it++){
//                    tmpMap.insert(it.key(),it.value());
////                    qDebug() << QString::fromLocal8Bit("key: %1\t value: %2\n").arg(it.key()).arg(it.value());
//                }
                QString filename = QString(wpMap.find("filename").value());
                QFile file(filename);
                if (!filename.endsWith("xml") && file.exists()) //slide show not append and file must exist!
                    wallpapersMap.insert(QString(wpMap.find("filename").value()), wpMap);
//                qDebug() << QString::fromLocal8Bit("******结束元素<wallpaper>***") << wpList.length();
            }
            else if (elementnameStr == "wallpapers"){
//                qDebug() << QString::fromLocal8Bit("*******结束元素<wallpapers>****");
                break;
            }
        }
    }
}

void XmlHandle::_xmlwriter(QString targetname){
    QFile file(targetname);
    if (!file.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Error Open XML file: " << file.errorString();
        return;
    }

    QMap<QString, QString> currentheadMap;
    currentheadMap = (QMap<QString, QString>)wallpapersMap.find("head").value();

    QXmlStreamWriter writer;
    writer.setDevice(&file);
    writer.setAutoFormatting(true); //自动格式化
//    writer.setCodec(headMap.find("encoding"));
    writer.writeStartDocument(QString(currentheadMap.find("version").value()), false);

    //DTD
    writer.writeDTD(QString::fromLocal8Bit("<!DOCTYPE %1 SYSTEM \"%2\">").arg(currentheadMap.find("doctype").value()).arg(currentheadMap.find("system").value()));

    //BODY
    writer.writeStartElement("wallpapers");
    QMap<QString, QMap<QString, QString> >::iterator its = wallpapersMap.begin();
    for (; its != wallpapersMap.end(); its++){
        if (QString(its.key()) == "head") //跳过xml的头信息
            continue;

        QMap<QString, QString> sourceMap = (QMap<QString, QString>)its.value();
        QMap<QString, QString>::iterator it = sourceMap.begin();

        writer.writeStartElement("wallpaper");
        if (sourceMap.contains("deleted"))
            writer.writeAttribute("deleted", QString(sourceMap.find("deleted").value()));
        else
            writer.writeAttribute("deleted", "false");

        if (sourceMap.contains("artist"))
            writer.writeTextElement("artist", QString(sourceMap.find("artist").value()));
        else
            writer.writeTextElement("artist", "(none)");
        for(; it != sourceMap.end(); it++){
            if (it.key() == "deleted")
                continue;
            if (it.key() == "name")
                continue;
            if (it.key() == "name.zh_CN")
                writer.writeTextElement("name", QString(it.value()));
            else
                writer.writeTextElement(QString(it.key()), QString(it.value()));
        }
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    file.close();
}

void XmlHandle::xmlwriter(QString targetname, QMap<QString, QMap<QString, QString> > wallpaperinfosMap){
    QFile file(targetname);
    if (!file.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Error Open XML file: " << file.errorString();
        return;
    }

    QMap<QString, QString> currentheadMap;
    currentheadMap = (QMap<QString, QString>)wallpaperinfosMap.find("head").value();

    QXmlStreamWriter writer;
    writer.setDevice(&file);
    writer.setAutoFormatting(true); //自动格式化
//    writer.setCodec(headMap.find("encoding"));
    writer.writeStartDocument(QString(currentheadMap.find("version").value()), false);

    //DTD
    writer.writeDTD(QString::fromLocal8Bit("<!DOCTYPE %1 SYSTEM \"%2\">").arg(currentheadMap.find("doctype").value()).arg(currentheadMap.find("system").value()));

    //BODY
    writer.writeStartElement("wallpapers");
    QMap<QString, QMap<QString, QString> >::iterator its = wallpaperinfosMap.begin();
    for (; its != wallpaperinfosMap.end(); its++){
        if (QString(its.key()) == "head")
            continue;

        QMap<QString, QString> sourceMap = (QMap<QString, QString>)its.value();
        QMap<QString, QString>::iterator it = sourceMap.begin();

        writer.writeStartElement("wallpaper");
        writer.writeAttribute("deleted", QString(sourceMap.find("deleted").value()));

        for(; it != sourceMap.end(); it++){
            if (it.key() == "deleted")
                continue;
            writer.writeTextElement(QString(it.key()), QString(it.value()));
        }

        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndDocument();

    file.close();
}
