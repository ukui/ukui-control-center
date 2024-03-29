/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ksc_set_font_size.h"
ksc_set_font_size::ksc_set_font_size()
{


}

//QFont ksc_set_font_size::get_font(int type)
//{
//    switch (type) {
//    case 12:
//    {
//        QFont font;
//        font.setPixelSize(12);
//        return font;
//    }
//        break;
//    case 14:
//    {
//        QFont font;
//        font.setPixelSize(14);
//        return font;
//    }
//        break;

//    case 16:
//    {
//        QFont font;
//        font.setPixelSize(16);
//        font.setWeight(QFont::Medium);
//        return font;
//    }
//        break;

//    case 18:
//    {
//        QFont font;
//        font.setPixelSize(18);
//        font.setBold(true);
//        return font;
//    }
//        break;

//    case 20:
//    {
//        QFont font;
//        font.setPixelSize(20);
//        font.setBold(true);
//        return font;
//    }
//        break;

//    case 24:
//    {
//        QFont font;
//        font.setPixelSize(24);
//        font.setWeight(QFont::Medium);
//        return font;
//    }
//        break;

//    case 26:
//    {
//        QFont font;
//        font.setPixelSize(26);
//        font.setBold(true);
//        return font;
//    }
//        break;

//    case 30:
//    {
//        QFont font;
//        font.setPixelSize(30);
//        font.setBold(true);
//        return font;
//    }
//        break;
//    }
//}
int ksc_set_font_size::set_font(int type, QFont &font)
{
    if(type>50)
        return 0;
    font.setBold(false);
    switch (type) {
    case 12:
    {
        font.setPixelSize(12);
    }
        break;
    case 14:
    {
        font.setPixelSize(14);
    }
        break;

    case 16:
    {
        font.setPixelSize(16);
        font.setWeight(QFont::Medium);
    }
        break;

    case 18:
    {
        font.setPixelSize(18);
        font.setBold(true);
    }
        break;

    case 20:
    {

        font.setPixelSize(20);
        font.setBold(true);
    }
        break;

    case 24:
    {

        font.setPixelSize(24);
        font.setWeight(QFont::Medium);
    }
        break;

    case 26:
    {
        font.setPixelSize(26);
        font.setBold(true);
    }
        break;

    case 30:
    {
        font.setPixelSize(30);
        font.setBold(true);
    }
        break;
    }
    font.setPixelSize(type);
    return type;
}
int ksc_set_font_size::set_font(int type,QFont& font,QLabel* lable_lineedit)
{
    if(type>50)
        return 0;
    font.setPixelSize(type);
    font.setBold(false);
    switch (type) {
    case 12:
    {
        font.setPixelSize(12);
    }
        break;
    case 14:
    {
        font.setPixelSize(14);
    }
        break;

    case 16:
    {
        font.setPixelSize(16);
        font.setWeight(QFont::Medium);
    }
        break;

    case 18:
    {
        font.setPixelSize(18);
        //font.setBold(true);
    }
        break;

    case 20:
    {

        font.setPixelSize(20);
        font.setBold(true);
    }
        break;

    case 24:
    {

        font.setPixelSize(24);
        font.setWeight(QFont::Medium);
    }
        break;

    case 26:
    {
        font.setPixelSize(26);
        font.setBold(true);
    }
        break;

    case 30:
    {
        font.setPixelSize(30);
        font.setBold(true);
    }
        break;
    }
    lable_lineedit->setFont(font);
    return type;
}
int ksc_set_font_size::set_font(int type,QFont& font,QLineEdit* l_lineedit)
{
    if(type>50)
        return 0;
    font.setPixelSize(type);
    font.setBold(false);
    switch (type) {
    case 12:
    {
        font.setPixelSize(12);
    }
        break;
    case 14:
    {
        font.setPixelSize(14);
    }
        break;

    case 16:
    {
        font.setPixelSize(16);
        font.setWeight(QFont::Medium);
    }
        break;

    case 18:
    {
        font.setPixelSize(18);
        //font.setBold(true);
    }
        break;

    case 20:
    {

        font.setPixelSize(20);
        font.setBold(true);
    }
        break;

    case 24:
    {

        font.setPixelSize(24);
        font.setWeight(QFont::Medium);
    }
        break;

    case 26:
    {
        font.setPixelSize(26);
        font.setBold(true);
    }
        break;

    case 30:
    {
        font.setPixelSize(30);
        font.setBold(true);
    }
        break;
    }
    l_lineedit->setFont(font);
    return type;
}
int ksc_set_font_size::set_font(int type,QFont& font,QPushButton* m_Button)
{
    if(type>50)
        return 0;
    font.setPixelSize(type);
    font.setBold(false);
    switch (type) {
    case 12:
    {
        font.setPixelSize(12);
    }
        break;
    case 14:
    {
        font.setPixelSize(14);
    }
        break;

    case 16:
    {
        font.setPixelSize(16);
        font.setWeight(QFont::Medium);
    }
        break;

    case 18:
    {
        font.setPixelSize(18);
        //font.setBold(true);
    }
        break;

    case 20:
    {

        font.setPixelSize(20);
        font.setBold(true);
    }
        break;

    case 24:
    {

        font.setPixelSize(24);
        font.setWeight(QFont::Medium);
    }
        break;

    case 26:
    {
        font.setPixelSize(26);
        font.setBold(true);
    }
        break;

    case 30:
    {
        font.setPixelSize(30);
        font.setBold(true);
    }
        break;
    }
    m_Button->setFont(font);
    return type;
}
