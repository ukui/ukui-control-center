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

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>

#include <KF5/KScreen/kscreen/output.h>

class QVBoxLayout;
class OutputConfig;
class UnifiedOutputConfig;

class QLabel;
class QCheckBox;
class QSlider;
class QComboBox;

const QString kSession = "wayland";

class ControlPanel : public QFrame
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel() override;

    void setConfig(const KScreen::ConfigPtr &config);
    void setUnifiedOutput(const KScreen::OutputPtr &output);
    void activateOutputNoParam();

private:
    void isWayland();

public Q_SLOTS:
    void activateOutput(const KScreen::OutputPtr &output);
    void slotOutputConnectedChanged();

Q_SIGNALS:
    void changed();
    void scaleChanged(double scale);

private Q_SLOTS:
    void addOutput(const KScreen::OutputPtr &output, bool connectChanged);
    void removeOutput(int outputId);

public:
    QVBoxLayout *mLayout;

private:
    KScreen::ConfigPtr mConfig;
    QList<OutputConfig *> mOutputConfigs;

    UnifiedOutputConfig *mUnifiedOutputCfg;

    KScreen::OutputPtr mCurrentOutput;

    bool mIsWayland;
};

#endif // CONTROLPANEL_H
