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
#include "changtime.h"
#include "ui_changtime.h"

#include <QDebug>


const int BEGINYEAR = 1900;
const int BEGINMD = 1;

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangtimeDialog::ChangtimeDialog(bool hour,QWidget *parent) :m_isEFHour(hour),
    QDialog(parent),
    ui(new Ui::changtimedialog)
{

//    QFile QssFile("://combox.qss");
//    QssFile.open(QFile::ReadOnly);

//    if (QssFile.isOpen()){
//        qss = QLatin1String(QssFile.readAll());
//        QssFile.close();
//    }

    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    ui->widget_2->setAttribute(Qt::WA_StyledBackground,true);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

//    ui->frame->setStyleSheet("QFrame{background: #ffffff; border: none; border-radius: 6px;}");
    //关闭按钮在右上角，窗体radius 6px，所以按钮只得6px
    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
    ui->timeFrame->setStyleSheet("QFrame{border-radius: 8px; background: palette(Base);}");
    ui->dateFrame->setStyleSheet("QFrame{border-radius: 8px; background: palette(Base);}");
//    ui->closeBtn->setStyleSheet("QPushButton#closeBtn{background: #ffffff; border: none; border-radius: 6px;}"
//                                "QPushButton:hover:!pressed#closeBtn{background: #FA6056; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}"
//                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}");

    m_datetimeInterface = new QDBusInterface("org.freedesktop.timedate1",
                                       "/org/freedesktop/timedate1",
                                       "org.freedesktop.timedate1",
                                       QDBusConnection::systemBus());
    //初始化时间
    QDateTime current = QDateTime::currentDateTime();
    QString currenthourStr = current.toString("hh");
    QString currentminStr = current.toString("mm");
    QString currentsecStr = current.toString("ss");
    ui->time->setText(QString("%1 : %2 : %3").arg(currenthourStr).arg(currentminStr).arg(currentsecStr));
    m_hour = currenthourStr.toInt();
    m_minute = currentminStr.toInt();
    m_second = currentsecStr.toInt();

    //初始化日期
    int year = current.toString("yyyy").toInt();
    int month = current.toString("MM").toInt();
    int day = current.toString("dd").toInt();
    if(month < 10 && day < 10){
        ui->date->setText(QString("%1-0%2-0%3").arg(year).arg(month).arg(day));
    } else if(month < 10){
        ui->date->setText(QString("%1-0%2-%3").arg(year).arg(month).arg(day));
    } else if(day < 10){
        ui->date->setText(QString("%1-%2-0%3").arg(year).arg(month).arg(day));
    } else {
        ui->date->setText(QString("%1-%2-%3").arg(year).arg(month).arg(day));
    }
    m_year = year;
    m_month = month;
    m_day = day;

    initUi();
//    initStatus();

    m_chtimer = new QTimer();
    m_chtimer->start(1000);
    connect(m_chtimer, SIGNAL(timeout()), this, SLOT(datetimeUpdateSlot()));


//    connect(ui->monthcomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(dayUpdateSlot()));
//    connect(ui->yearcomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(dayUpdateSlot()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->confirmButton,SIGNAL(clicked()),this,SLOT(changtimeApplySlot()));
    connect(ui->closeBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });

}

ChangtimeDialog::~ChangtimeDialog()
{
    m_chtimer->stop();
    delete ui;
    delete m_datetimeInterface;
}


void ChangtimeDialog::datetimeUpdateSlot(){
    QDateTime current = QDateTime::currentDateTime();
    QString currenthourStr = current.toString("hh");
    QString currentminStr = current.toString("mm");
    QString currentsecStr = current.toString("ss");
    m_hour = currenthourStr.toInt();
    m_minute = currentminStr.toInt();
    m_second = currentsecStr.toInt();
    ui->time->setText(QString("%1 : %2 : %3").arg(currenthourStr).arg(currentminStr).arg(currentsecStr));
//    ui->seccomboBox->setCurrentIndex(currentsecStr.toInt());
//    if (currentsecStr.toInt() == 0) {
//       ui->mincomboBox->setCurrentIndex(currentminStr.toInt());
//    }
//    if (currentsecStr.toInt() == 0 && currentminStr.toInt() == 0) {
//        ui->hourcomboBox->setCurrentIndex(currenthourStr.toInt());
//    }
}


void ChangtimeDialog::changtimeApplySlot(){
//    qDebug()<<"时间应用------------》"<<endl;
    emit time_changed();
    int year = m_year;
    int month = m_month;
    int day = m_day;

    QDate tmpdate(year,month,day);

    int hour = m_hour;
    int min = m_minute;
    int second = m_second;

    QTime tmptime(hour, min,second);

    QDateTime setdt(tmpdate,tmptime);
//    qDebug()<<"tmp time and hour is-->"<<setdt<<" "<<hour<<endl;

    for(int i=0; i < 2; i++){
        m_datetimeInterface->call("SetNTP", false, true);//先关闭网络同步
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
        m_datetimeInterface->call("SetTime", QVariant::fromValue(setdt.toMSecsSinceEpoch() / 1000 * G_TIME_SPAN_SECOND), false, true);
#else
        m_datetimeInterface->call("SetLocalRTC",true,false,true);
        m_datetimeInterface->call("SetTime", QVariant::fromValue(setdt.toSecsSinceEpoch() * G_TIME_SPAN_SECOND), false, true);
#endif
    }
    this->close();
}

void ChangtimeDialog::initUi(){
    ui->timeLabel->setText(tr("time"));
    ui->dateLabel->setText(tr("date"));

    QLabel * timeBtnLabel = new QLabel(ui->timeBtn);
    timeBtnLabel->setScaledContents(true);
    timeBtnLabel->setPixmap(QPixmap(":/img/plugins/datetime/time.png"));
    QLabel * dateBtnLabel = new QLabel(ui->dateBtn);
    dateBtnLabel->setScaledContents(true);
    dateBtnLabel->setPixmap(QPixmap(":/img/plugins/datetime/date.svg"));

    ui->timeBtn->setStyleSheet("QPushButton{border-radius: 4px;}");
    ui->timeBtn->setAutoFillBackground(false);
    ui->timeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->dateBtn->setStyleSheet("QPushButton{border-radius: 4px;}");
    ui->dateBtn->setAutoFillBackground(false);
    ui->dateBtn->setCursor(QCursor(Qt::PointingHandCursor));

    QColor dColorCurrentText(palette().color(QPalette::Text));
    QColor dColorLine("#F6F6F6");
    QColor dColorDisableText("#999999");
    QColor dColorBackground(palette().color(QPalette::Base));

    //创建日期选择器
    QWidget *baseWidget_2 = new QWidget(this);
    baseWidget_2->setObjectName("baseWidget_2");
    baseWidget_2->setStyleSheet("QWidget#baseWidget_2{background: palette(Base);\
                                 border: 1px solid #DDDDDD;\
                                 border-radius: 8px;}");
    QWidget * chooseWidget = new QWidget(baseWidget_2);
    QWidget * yearWidget = new QWidget();
    QWidget * monthWidget = new QWidget();
    QWidget * dayWidget = new QWidget();
    QFrame * line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);

    QHBoxLayout * chooseLayout = new QHBoxLayout(chooseWidget);
    yearWidget->setFixedSize(77,100);
    monthWidget->setFixedSize(77,100);
    dayWidget->setFixedSize(77,100);
    chooseWidget->setFixedHeight(100);
    chooseLayout->addWidget(yearWidget);
    chooseLayout->addWidget(monthWidget);
    chooseLayout->addWidget(dayWidget);
    chooseLayout->setContentsMargins(0, 0, 0, 0);
    chooseLayout->setSpacing(0);
    chooseWidget->setLayout(chooseLayout);

    QWidget * btnWidget = new QWidget(baseWidget_2);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    QPushButton * todayBtn = new QPushButton();
    QPushButton * cancelBtn = new QPushButton();
    QPushButton * confirmBtn = new QPushButton();
    todayBtn->setText(tr("today"));
    cancelBtn->setText(tr("cancel"));
    confirmBtn->setText(tr("confirm"));
    todayBtn->setStyleSheet("QPushButton{background: palette(Base);color: palette(Text);}"
                            "QPushButton:hover:!pressed{color: #FFFFFF;}"
                            "QPushButton:hover:pressed{color: #FFFFFF;}");
    cancelBtn->setStyleSheet("QPushButton{background-color: palette(Base);color: palette(Text);}"
                             "QPushButton:hover:!pressed{color: #FFFFFF;}"
                             "QPushButton:hover:pressed{color: #FFFFFF;}");
    confirmBtn->setStyleSheet("QPushButton{background: palette(Base);color: #2FB3E8;}"
                              "QPushButton:hover:!pressed{color: #FFFFFF;}"
                              "QPushButton:hover:pressed{color: #FFFFFF;}");
    todayBtn->setFixedSize(60, 30);
    cancelBtn->setFixedSize(60, 30);
    confirmBtn->setFixedSize(60, 30);
    btnLayout->addWidget(todayBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn);
    btnLayout->setContentsMargins(9,0,9,0);
    btnWidget->setFixedHeight(30);
    btnWidget->setLayout(btnLayout);

    connect(todayBtn, &QPushButton::clicked, this, [=](){
        pWidgetYear->setValue(m_year);
        pWidgetMonth->setValue(m_month);
        changeMonthSlot();
        pWidgetDay->setValue(m_day);
        pWidgetYear->repaint();
        pWidgetMonth->repaint();
        pWidgetDay->repaint();
    });
    connect(cancelBtn, &QPushButton::clicked, this, [=](){
        ui->dateFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
        baseWidget_2->setVisible(false);
    });
    connect(confirmBtn, &QPushButton::clicked, this, [=](){
        m_year = pWidgetYear->getValue();
        m_month = pWidgetMonth->getValue();
        m_day = pWidgetDay->getValue();
        if(m_month < 10 && m_day < 10){
            ui->date->setText(QString("%1-0%2-0%3").arg(m_year).arg(m_month).arg(m_day));
        } else if(m_month < 10){
            ui->date->setText(QString("%1-0%2-%3").arg(m_year).arg(m_month).arg(m_day));
        } else if(m_day < 10){
            ui->date->setText(QString("%1-%2-0%3").arg(m_year).arg(m_month).arg(m_day));
        } else {
            ui->date->setText(QString("%1-%2-%3").arg(m_year).arg(m_month).arg(m_day));
        }
        ui->dateFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
        baseWidget_2->setVisible(false);
    });

    QVBoxLayout * mainLayout_2 = new QVBoxLayout(baseWidget_2);
    mainLayout_2->addWidget(chooseWidget);
    mainLayout_2->addWidget(line);
    mainLayout_2->addWidget(btnWidget);
    mainLayout_2->setContentsMargins(0,9,0,9);
    mainLayout_2->setSpacing(6);
    baseWidget_2->setLayout(mainLayout_2);
    baseWidget_2->setFixedHeight(184);
    baseWidget_2->setGeometry(116, 221, 236, 150);
    baseWidget_2->setVisible(false);
    baseWidget_2->setAutoFillBackground(true);

    pWidgetYear = new ScrollTimeChooser(yearWidget);
    pWidgetYear->setRang(1900, 2100);
    pWidgetYear->setValue(m_year);
    pWidgetYear->setColor(dColorCurrentText);
    pWidgetYear->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetYear->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetYear->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);

    pWidgetMonth = new ScrollTimeChooser(monthWidget);
    pWidgetMonth->setRang(1, 12);
    pWidgetMonth->setValue(m_month);
    pWidgetMonth->setColor(dColorCurrentText);
    pWidgetMonth->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetMonth->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetMonth->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);
    connect(pWidgetMonth, SIGNAL(signal_currentValueChange()), this, SLOT(changeMonthSlot()));

    qDebug()<<pWidgetMonth->getValue();
    pWidgetDay = new ScrollTimeChooser(dayWidget);
    if(pWidgetMonth->getValue() == 4 || pWidgetMonth->getValue() == 6 ||
       pWidgetMonth->getValue() == 9 || pWidgetMonth->getValue() == 11) {
        pWidgetDay->setRang(1, 30);
    } else if(pWidgetMonth->getValue() == 2){
        if(!(pWidgetYear->getValue()%400) || (!(pWidgetYear->getValue()%4) && pWidgetYear->getValue()%100)){
            pWidgetDay->setRang(1, 29);
        } else {
            pWidgetDay->setRang(1, 28);
        }
    } else {
        pWidgetDay->setRang(1, 31);
    }
    pWidgetDay->setValue(m_day);
    pWidgetDay->setColor(dColorCurrentText);
    pWidgetDay->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetDay->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetDay->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);

    connect(ui->dateBtn, &QPushButton::clicked, this, [=]{
        if(baseWidget_2->isVisible()) {
            ui->dateFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
            baseWidget_2->setVisible(false);
        } else {
            ui->dateFrame->setStyleSheet("QFrame#dateFrame{border: 2px solid #2FB3E8; border-radius: 8px; background: palette(Base);}");
            baseWidget_2->setVisible(true);
        }
    });

    //创建时间选择器
    QWidget * baseWidget = new QWidget(this);
    baseWidget->setObjectName("baseWidget_2");
    baseWidget->setStyleSheet("QWidget#baseWidget_2{background: palette(Base);\
                               border: 1px solid #DDDDDD;\
                               border-radius: 8px;}");
    QWidget * chooseWidget_2 = new QWidget(baseWidget);
    QWidget * btnWidget_2 = new QWidget(baseWidget);
    QWidget * hourWidget = new QWidget();
    QWidget * minWidget = new QWidget();
    QWidget * secondWidget = new QWidget();
    QFrame * line_2 = new QFrame;
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFixedHeight(1);

    QHBoxLayout * chooseLayout_2 = new QHBoxLayout(chooseWidget_2);
    hourWidget->setFixedSize(77,100);
    minWidget->setFixedSize(77,100);
    secondWidget->setFixedSize(77,100);
    chooseWidget_2->setFixedHeight(100);
    chooseLayout_2->addWidget(hourWidget);
    chooseLayout_2->addWidget(minWidget);
    chooseLayout_2->addWidget(secondWidget);
    chooseLayout_2->setContentsMargins(0, 0, 0, 0);
    chooseLayout_2->setSpacing(0);
    chooseWidget_2->setLayout(chooseLayout_2);

    QHBoxLayout * btnLayout_2 = new QHBoxLayout(btnWidget_2);
    QPushButton * cancelBtn_2 = new QPushButton();
    QPushButton * confirmBtn_2 = new QPushButton();
    cancelBtn_2->setText(tr("cancel"));
    confirmBtn_2->setText(tr("confirm"));
    cancelBtn_2->setFixedSize(60, 30);
    confirmBtn_2->setFixedSize(60, 30);
    btnLayout_2->addStretch();
    btnLayout_2->addWidget(cancelBtn_2);
    btnLayout_2->addWidget(confirmBtn_2);
    btnLayout_2->setSpacing(16);
    btnLayout_2->setContentsMargins(9,0,9,0);
    btnWidget_2->setFixedHeight(30);
    btnWidget_2->setLayout(btnLayout_2);
    cancelBtn_2->setStyleSheet("QPushButton{background-color: palette(Base);color: palette(Text);}"
                             "QPushButton:hover:!pressed{color: #FFFFFF;}"
                             "QPushButton:hover:pressed{color: #FFFFFF;}");
    confirmBtn_2->setStyleSheet("QPushButton{background: palette(Base);color: #2FB3E8;}"
                              "QPushButton:hover:!pressed{color: #FFFFFF;}"
                              "QPushButton:hover:pressed{color: #FFFFFF;}");

    connect(cancelBtn_2, &QPushButton::clicked, this, [=](){
        ui->timeFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
        baseWidget->setVisible(false);
    });
    connect(confirmBtn_2, &QPushButton::clicked, this, [=](){
        m_chtimer->stop();
        m_hour = pWidgetHour->getValue();
        m_minute = pWidgetMin->getValue();
        m_second = pWidgetSecond->getValue();
        ui->time->setText(QString("%1 : %2 : %3").arg(m_hour).arg(m_minute).arg(m_second));
        ui->timeFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
        baseWidget->setVisible(false);
    });

    QVBoxLayout * mainLayout = new QVBoxLayout(baseWidget);
    mainLayout->addWidget(chooseWidget_2);
    mainLayout->addWidget(line_2);
    mainLayout->addWidget(btnWidget_2);
    mainLayout->setContentsMargins(0,6,0,6);
    mainLayout->setSpacing(6);
    baseWidget->setLayout(mainLayout);
    baseWidget->setFixedHeight(184);
    baseWidget->setGeometry(116, 142, 236, 150);
    baseWidget->setVisible(false);
    baseWidget->setAutoFillBackground(true);
    pWidgetHour = new ScrollTimeChooser(hourWidget);
    pWidgetHour->setRang(0, 23);
    pWidgetHour->setValue(m_hour);
    pWidgetHour->setColor(dColorCurrentText);
    pWidgetHour->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetHour->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetHour->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);

    pWidgetMin = new ScrollTimeChooser(minWidget);
    pWidgetMin->setRang(0, 59);
    pWidgetMin->setValue(m_minute);
    pWidgetMin->setColor(dColorCurrentText);
    pWidgetMin->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetMin->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetMin->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);

    pWidgetSecond = new ScrollTimeChooser(secondWidget);
    pWidgetSecond->setRang(0, 59);
    pWidgetSecond->setValue(m_second);
    pWidgetSecond->setColor(dColorCurrentText);
    pWidgetSecond->setColor(dColorLine, ScrollTimeChooser::ColorType::LINE);
    pWidgetSecond->setColor(dColorDisableText, ScrollTimeChooser::ColorType::DISABLETEXT);
    pWidgetSecond->setColor(dColorBackground, ScrollTimeChooser::ColorType::BACKHROUND);

    connect(ui->timeBtn, &QPushButton::clicked, this, [=]{
        if(baseWidget->isVisible()) {
            ui->timeFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
            baseWidget->setVisible(false);
        } else {
            if(baseWidget_2->isVisible()){
                ui->dateFrame->setStyleSheet("QFrame{border: none; border-radius: 8px; background: palette(Base);}");
                baseWidget_2->setVisible(false);
            }
            ui->timeFrame->setStyleSheet("QFrame#timeFrame{border: 2px solid #2FB3E8; border-radius: 8px; background: palette(Base);}");
            pWidgetSecond->setValue(m_second);
            baseWidget->setVisible(true);
        }
    });
}

void ChangtimeDialog::changeMonthSlot(){
    int year = pWidgetYear->getValue();
    int month = pWidgetMonth->getValue();
    int day = pWidgetDay->getValue();
    if(month == 4 || month == 6 || month == 9 || month == 11){
        if(day > 30){
            pWidgetDay->setValue(30);
        }
        pWidgetDay->setRang(1, 30);
    } else if(month == 2){
        if(!(year%400) || (!(year%4) && (year%100))) {
            if(day > 29){
                pWidgetDay->setValue(29);
            }
            pWidgetDay->setRang(1, 29);
        } else {
            if(day > 28){
                pWidgetDay->setValue(28);
            }
            pWidgetDay->setRang(1, 28);
        }
    } else {
        pWidgetDay->setRang(1, 31);
    }
    pWidgetDay->repaint();
}


void ChangtimeDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -74), 6, 6);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
//    p.fillPath(rectPath,QColor(0,0,0));
    p.restore();
}
