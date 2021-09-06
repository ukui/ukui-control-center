#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QApplication>
#include <QTranslator>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyleOption>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QDialog>

class MessageBoxPowerIntel : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBoxPowerIntel(QWidget *parent = nullptr);
    ~MessageBoxPowerIntel();
    QHBoxLayout  *m_pfirstlayout    = nullptr;
    QVBoxLayout  *m_pmainlayout     = nullptr;
    QVBoxLayout  *m_ptestlayout     = nullptr;
    QHBoxLayout  *buttonlayout      = nullptr;
    QHBoxLayout  *m_pcenterlayout     = nullptr;

    QWidget *topWidget;
    QWidget *contentWidget;
    QWidget *buttonWidget;
    QWidget *leftIconWidget;
    QWidget *centerWidget;


    QPushButton *closeButton;



    QLabel *firstlabel;

    QPushButton *remindButton;
    QPushButton *confirmButton;

    QLabel *iconBigLabel;

    QTimer *timer;

    void initUI();
    void paintEvent(QPaintEvent *e);

public Q_SLOTS:
    void otaevent();
    void timetask();

protected slots:
    void remindslots();
    void restartslots();
};
#endif // WIDGET_H
