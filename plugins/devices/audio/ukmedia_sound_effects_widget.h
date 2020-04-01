#ifndef UKMEDIASOUNDEFFECTSWIDGET_H
#define UKMEDIASOUNDEFFECTSWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QStyledItemDelegate>

class UkmediaSoundEffectsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaSoundEffectsWidget(QWidget *parent = nullptr);
    ~UkmediaSoundEffectsWidget();
    friend class UkmediaMainWidget;
Q_SIGNALS:

public Q_SLOTS:

private:
    QWidget *m_pThemeWidget;
    QWidget *m_pShutDownWidget;
    QWidget *m_pLagoutWidget;
    QString qss;
    QStyledItemDelegate *itemDelegate;
    QLabel *m_pSoundEffectLabel;
    QLabel *m_pSoundThemeLabel;
    QLabel *m_pShutdownlabel;
    QLabel *m_pLagoutLabel;
    QComboBox *m_pSoundThemeCombobox;
    QComboBox *m_pShutdownCombobox;
    QComboBox *m_pLagoutCombobox;
};

#endif // UKMEDIASOUNDEFFECTSWIDGET_H
