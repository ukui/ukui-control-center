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

#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>
#include <QWidget>

#include <KF5/KScreen/kscreen/output.h>

#include <QGSettings>



class QCheckBox;
class ResolutionSlider;
class QLabel;
class QStyledItemDelegate;

namespace Ui {
class KScreenWidget;
}

class OutputConfig : public QWidget
{
    Q_OBJECT

public:
    explicit OutputConfig(QWidget *parent);
    explicit OutputConfig(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~OutputConfig() override;

    virtual void setOutput(const KScreen::OutputPtr &output);
    KScreen::OutputPtr output() const;

    void setTitle(const QString &title);
    void setShowScaleOption(bool showScaleOption);
    bool showScaleOption() const;

    void initConfig(const KScreen::ConfigPtr &config);

protected Q_SLOTS:
    void slotResolutionChanged(const QSize &size, bool emitFlag);
    void slotRotationChanged(int index);
    void slotRefreshRateChanged(int index);
    void slotScaleChanged(int index);
    void slotEnableWidget();

Q_SIGNALS:
    void changed();
    void scaleChanged(double scale);

protected:
    virtual void initUi();

private:
    void initConnection();
    QString scaleToString(double scale);

protected:
    KScreen::OutputPtr mOutput;
    QLabel *mTitle = nullptr;
    QCheckBox *mEnabled = nullptr;
    ResolutionSlider *mResolution = nullptr;

    QComboBox *mRotation = nullptr;
    QComboBox *mScale = nullptr;
    QComboBox *mRefreshRate = nullptr;
    QComboBox *mMonitor = nullptr;
    QComboBox *mScaleCombox = nullptr;

    bool mShowScaleOption = false;
    bool mIsFirstLoad = true;

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    KScreen::ConfigPtr mConfig;
#else
    KScreen::ConfigPtr mConfig = nullptr;
#endif

    QGSettings *mDpiSettings = nullptr;
};

#endif // OUTPUTCONFIG_H
