#include "dataformat.h"
#include "ui_dataformat.h"

#include <QDateTime>
#include <QDebug>

#define PANEL_GSCHEMAL   "org.ukui.panel.plugins"
#define CALENDAR_KEY     "calendar"
#define DAY_KEY          "firstday"
#define DATE_FORMATE_KEY "date"
#define TIME_KEY         "time"

DataFormat::DataFormat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataFormat)
{
    ui->setupUi(this);
    QByteArray id(PANEL_GSCHEMAL);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id)        ;

    }

    initUi();
    initConnect();
    initComponent();
}

DataFormat::~DataFormat()
{
    delete ui;
    delete m_itimer;
}


void DataFormat::initUi() {
    ui->calendarBox->addItem(tr("lunar"));
    ui->calendarBox->addItem(tr("solar calendar"));

    ui->dayBox->addItem(tr("monday"));
    ui->dayBox->addItem(tr("sunday"));

    QString currentsecStr;
    QDateTime current = QDateTime::currentDateTime();

    currentsecStr = current.toString("yyyy/MM/dd ");
    ui->dateBox->addItem(currentsecStr);

    currentsecStr = current.toString("yyyy-MM-dd ");
    ui->dateBox->addItem(currentsecStr);

    QString timeStr;
    timeStr = current.toString("hh: mm : ss");
    ui->timeBox->addItem(timeStr);

    timeStr = current.toString("AP hh: mm : ss");
    ui->timeBox->addItem(timeStr);
}

void DataFormat::initConnect() {
    m_itimer = new QTimer();
    m_itimer->start(1000);

    connect(m_itimer,SIGNAL(timeout()), this, SLOT(datetime_update_slot()));
    connect(ui->calendarBox, SIGNAL(currentIndexChanged(int)), SLOT(calendar_change_slot(int)));
    connect(ui->dayBox, SIGNAL(currentIndexChanged(int)), SLOT(day_change_slot(int)));
    connect(ui->dateBox, SIGNAL(currentIndexChanged(int)), SLOT(date_change_slot(int)));
    connect(ui->timeBox, SIGNAL(currentIndexChanged(int)), SLOT(time_change_slot(int)));
}

void DataFormat::initComponent() {
    if (!m_gsettings) {
        return ;
    }
    const QStringList list = m_gsettings->keys();

    if (!list.contains("calendar") || !list.contains("firstday")){
        return ;
    }

    QString clac = m_gsettings->get(CALENDAR_KEY).toString();
    if ("lunar" == clac) {
        ui->calendarBox->setCurrentIndex(0);
    } else {
        ui->calendarBox->setCurrentIndex(1);
    }

    QString day = m_gsettings->get(DAY_KEY).toString();
    if ("monday" == day) {
        ui->dayBox->setCurrentIndex(0);
    } else {
        ui->dayBox->setCurrentIndex(1);
    }

    QString dateFormat = m_gsettings->get(DATE_FORMATE_KEY).toString();
    if ("cn" == dateFormat) {
       ui->timeBox->setCurrentIndex(0);
    } else {
       ui->timeBox->setCurrentIndex(1);
    }
}

void DataFormat::datetime_update_slot() {
    QString timeStr;
    QDateTime current = QDateTime::currentDateTime();

    timeStr = current.toString("hh: mm : ss");
    ui->timeBox->setItemText(0,timeStr);

    timeStr = current.toString("AP hh: mm : ss");
    ui->timeBox->setItemText(1,timeStr);
}

void DataFormat::calendar_change_slot(int index) {

}

void DataFormat::day_change_slot(int index) {

}

void DataFormat::date_change_slot(int index) {

}

void DataFormat::time_change_slot(int index) {

}
