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
#include "bgfileparse.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamReader>

#include <QDebug>

QMap<QString, BgInfo> wholeBgInfo;

BgFileParse::BgFileParse()
{
}

BgFileParse::~BgFileParse()
{
}


QMap<QString, BgInfo> BgFileParse::bgFileReader(){
    QString filename = QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("wallpaper.xml");

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        QMap<QString, BgInfo> nMap;
        qDebug() << "Error Open XML File When Reader Xml: " << file.errorString();
        return nMap;
    }

    QXmlStreamReader reader;
    reader.setDevice(&file);

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType nType = reader.readNext();
        switch (nType) {
        case QXmlStreamReader::StartDocument: {
            QString versionStr = reader.documentVersion().toString();
            QString encodingStr = reader.documentEncoding().toString();
//            bool aloneBool = reader.isStandaloneDocument();
//            qDebug() << QString::fromLocal8Bit("Version: %1 encoding: %2 standalone: %3").arg(versionStr).arg(encodingStr).arg(aloneBool);
            break;
        }
        case QXmlStreamReader::DTD: {
            QString dtdStr = reader.text().toString();
            QString dtdnameStr = reader.dtdName().toString();
            QString dtdsystemidStr = reader.dtdSystemId().toString();
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
                parseWallpaper(reader);
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

    if (reader.hasError()){
        qDebug() << QString::fromLocal8Bit("msg: %1; line: %2; column: %3; char shift: %4").arg(reader.errorString()).arg(reader.lineNumber()).arg(reader.columnNumber()).arg(reader.characterOffset());
    }
    file.close();

    return wholeBgInfo;
}


void BgFileParse::parseWallpaper(QXmlStreamReader &reader){
    BgInfo oneBgInfo;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()){
            QString elementnameStr = reader.name().toString();
            if (elementnameStr == "wallpaper"){
//                qDebug() << QString::fromLocal8Bit("******开始元素<wallpaper>***");
                QXmlStreamAttributes wp_attributes = reader.attributes();
                if (wp_attributes.hasAttribute("deleted")){
                    QString deletedStr = wp_attributes.value("deleted").toString();
                    oneBgInfo.status = deletedStr;
                }
            } else if (elementnameStr == "name"){
                QXmlStreamAttributes name_attributes = reader.attributes();
                if (name_attributes.hasAttribute("xml:lang")){
                    QString langStr = name_attributes.value("xml:lang").toString();
//                    qDebug() << QString::fromLocal8Bit("name zh_CN: %1").arg(reader.readElementText());
                    oneBgInfo.i18nName = reader.readElementText();
                } else {
                    oneBgInfo.name = reader.readElementText();
                }
            } else if (elementnameStr == "artist"){
                oneBgInfo.artist = reader.readElementText();
            } else if (elementnameStr == "filename"){
                oneBgInfo.filename = reader.readElementText();
            } else if (elementnameStr == "options"){
                oneBgInfo.option = reader.readElementText();
            } else if (elementnameStr == "pcolor"){
                oneBgInfo.pColor = reader.readElementText();
            } else if (elementnameStr == "scolor"){
                oneBgInfo.sColor = reader.readElementText();
            } else if (elementnameStr == "shade_type"){
                oneBgInfo.shadeType = reader.readElementText();
            }
        }
        else if (reader.isEndElement()){
            QString elementnameStr = reader.name().toString();
            if (elementnameStr == "wallpaper"){
                QString filename = oneBgInfo.filename;
                QFile file(filename);
                //slide show not append and file must exist!
                if (!filename.endsWith("xml") && file.exists())
                    wholeBgInfo.insert(oneBgInfo.filename, oneBgInfo);
//                qDebug() << QString::fromLocal8Bit("******结束元素<wallpaper>***") << wpList.length();
            }
            else if (elementnameStr == "wallpapers"){
//                qDebug() << QString::fromLocal8Bit("*******结束元素<wallpapers>****");
                break;
            }
        }
    }
}
