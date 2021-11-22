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

#ifndef RESOLUTIONSLIDER_H
#define RESOLUTIONSLIDER_H

#include <QWidget>
#include <QSet>

#include <KF5/KScreen/kscreen/output.h>

class QSlider;
class QLabel;
class QComboBox;
class QStyledItemDelegate;

class ResolutionSlider : public QWidget
{
    Q_OBJECT

public:
    explicit ResolutionSlider(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~ResolutionSlider() override;

    QSize currentResolution() const;
    QSize getMaxResolution() const;

    void setResolution(const QSize &size);

Q_SIGNALS:
    void resolutionChanged(const QSize &size, bool emitFlag = true);
    void resolutionsave(const QSize &size);

public Q_SLOTS:
    void slotValueChanged(int);
    void slotOutputModeChanged();

private:
    void init();

private:
    KScreen::OutputPtr mOutput;

    QList<QSize> mModes;
    QList<QSize> mExcludeModes;

    QComboBox *mComboBox = nullptr;

    bool mIsWayland = false;
};

#endif // RESOLUTIONSLIDER_H
