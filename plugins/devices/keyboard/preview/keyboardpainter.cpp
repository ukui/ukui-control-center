/*
 *  Copyright (C) 2012 Shivam Makkar (amourphious1992@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "keyboardpainter.h"
#include "geometry_components.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>


KeyboardPainter::KeyboardPainter():
    kbDialog(new QDialog(this)),
    kbframe(new KbPreviewFrame(this)),
    exitButton(new QPushButton(tr("Close"), this)),
    levelBox(new QComboBox(this))
{
    this->setFixedSize(1250, 600);
    kbframe->setFixedSize(1100, 490);
    exitButton->setFixedSize(120, 30);
    levelBox->setFixedSize(360, 30);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout();

    hLayout->addWidget(exitButton, 0, Qt::AlignLeft);
    hLayout->addWidget(levelBox, 0, Qt::AlignRight);
    hLayout->addSpacing(30);

    vLayout->addWidget(kbframe);
    vLayout->addLayout(hLayout);

    connect(exitButton, &QPushButton::clicked, this, &KeyboardPainter::close);
    connect(levelBox, SIGNAL(activated(int)), this, SLOT(levelChanged(int)));

    setWindowTitle(kbframe->getLayoutName());
    levelBox->setVisible(false);
}


void KeyboardPainter::generateKeyboardLayout(const QString &layout, const QString &variant, const QString &model, const QString &title)
{
    kbframe->generateKeyboardLayout(layout, variant, model);
    kbframe->setFixedSize(getWidth(), getHeight());
    kbDialog->setFixedSize(getWidth(), getWidth());
    setWindowTitle(title);

    int level = kbframe->getLevel();

    if (level > 4) {
        levelBox->addItem(tr("Keyboard layout levels"), (tr("Level %1, %2").arg(3, 4)));
        for (int i = 5; i <= level; i += 2) {
            levelBox->addItem(tr("Keyboard layout levels"), (tr("Level %1, %2").arg(i, i + 1)));
        }
    } else {
        levelBox->setVisible(false);
    }
}

void KeyboardPainter::levelChanged(int l_id)
{
    kbframe->setL_id(l_id);
}

int KeyboardPainter::getHeight()
{
    int height = kbframe->getHeight();
    height = kbframe->getScaleFactor() * height + 50;
    return height;
}

int KeyboardPainter::getWidth()
{
    int width = kbframe->getWidth();
    width = kbframe->getScaleFactor() * width + 20;
    return width;
}

KeyboardPainter::~KeyboardPainter()
{
    delete kbframe;
    kbframe = nullptr;
    delete exitButton;
    exitButton = nullptr;
    delete levelBox;
    levelBox = nullptr;
}
