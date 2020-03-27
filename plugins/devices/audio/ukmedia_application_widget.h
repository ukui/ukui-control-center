#ifndef UKMEDIAAPPLICATIONWIDGET_H
#define UKMEDIAAPPLICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "ukmedia_output_widget.h"
class UkmediaApplicationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaApplicationWidget(QWidget *parent = nullptr);
    ~UkmediaApplicationWidget();
    friend class UkmediaMainWidget;
Q_SIGNALS:

public Q_SLOTS:

private:
    QLabel *m_pApplicationLabel;
    QLabel *m_pNoAppLabel;
    QWidget *m_pAppWid;
    QStringList *m_pAppVolumeList;
    QLabel *m_pAppLabel;
    QLabel *m_pAppIconLabel;
    QPushButton *m_pAppIconBtn;
    QLabel *m_pAppVolumeLabel;
    AudioSlider *m_pAppSlider;
    QGridLayout *m_pGridlayout;
};

#endif // UKMEDIAAPPLICATIONWIDGET_H
