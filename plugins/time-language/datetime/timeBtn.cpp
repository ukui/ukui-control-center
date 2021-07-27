#include "timeBtn.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include "datetime.h"

TimeBtn::TimeBtn(const QString &timezone) {
    this->setProperty("useButtonPalette", true);
    this->setMinimumHeight(90);
    this->adjustSize();

    QHBoxLayout *timeShowLayout = new QHBoxLayout(this);
    QWidget     *timeWid      = new QWidget(this);
    QVBoxLayout *timeLayout   = new QVBoxLayout(timeWid);
    labelInfo    = new FixLabel(this);    //时间,和标题字号一致
    labelTime    = new FixLabel(this);    //日期
    deleteBtn    = new QPushButton(this);

    timeShowLayout->setContentsMargins(0,0,18,0);
    timeLayout->setContentsMargins(18,0,18,0);
    timeShowLayout->addWidget(timeWid);
    timeShowLayout->addWidget(deleteBtn);
    deleteBtn->setFixedSize(28,28);
    deleteBtn->setProperty("isWindowButton", 0x02);
    deleteBtn->setProperty("useIconHighlightEffect", 0x08);
    deleteBtn->setFlat(true);
    deleteBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    deleteBtn->setVisible(false);

    timeLayout->addStretch();
    timeLayout->addWidget(labelInfo);
    timeLayout->addWidget(labelTime);
    timeLayout->addStretch();
    labelInfo->setObjectName("DateTime_Info");
    labelTime->setObjectName("DateTime_Time");

    thisZone = QTimeZone(timezone.toLatin1().data());
    int utcOff = thisZone.offsetFromUtc(QDateTime::currentDateTime())/3600;
    QString gmData;
    if (utcOff >= 0) {
        gmData = QString("(GMT+%1:%2)").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    } else {
        gmData = QString("(GMT%1:%2)").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    }
    labelInfo->setText(DateTime::getLocalTimezoneName(timezone, QLocale::system().name()) + "   " + gmData);

    QFont font;
    QGSettings *m_fontSetting = new QGSettings("org.ukui.style");
    font.setFamily(m_fontSetting->get("systemFont").toString());
    font.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 18 / 11);  //设置的是pt，按照公式计算为px,标题默认字为18px
    font.setWeight(QFont::Medium);
    labelInfo->setFont(font);
}


TimeBtn::~TimeBtn() {

}

void TimeBtn::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    deleteBtn->setVisible(true);
}

void TimeBtn::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    deleteBtn->setVisible(false);
}

void TimeBtn::updateTime(bool hour_24) {
    QString localizedTimezone = "";
    QTimeZone currentZone  = QTimeZone(localizedTimezone.toLatin1().data());
    QString time;
    QDateTime thisZoneTime;
    const double timeDelta = (thisZone.offsetFromUtc(QDateTime::currentDateTime()) - currentZone.offsetFromUtc(QDateTime::currentDateTime())) / 3600.0;
    QString dateLiteral;
    if (QDateTime::currentDateTime().toTimeZone(thisZone).time().hour() + timeDelta >= 24) {
        dateLiteral = tr("Tomorrow");
    } else if (QDateTime::currentDateTime().toTimeZone(thisZone).time().hour() + timeDelta < 0) {
        dateLiteral = tr("Yesterday");
    } else {
        dateLiteral = tr("Today");
    }
    int decimalNumber = 1;
    //小时取余,再取分钟,将15分钟的双倍只显示一位小数,其他的都显示两位小数
    switch ((thisZone.offsetFromUtc(QDateTime::currentDateTime()) - currentZone.offsetFromUtc(QDateTime::currentDateTime())) / 3600 / 60 / 15) {
    case -1:
    case -3:
    case 1:
    case 3:
        decimalNumber = 2;
        break;
    default:
        decimalNumber = 1;
        break;
    }

    QString compareLiteral;
    if (timeDelta > 0) {
        compareLiteral = tr("%1 hours earlier than local").arg(QString::number(timeDelta, 'f', decimalNumber));
    } else {
        compareLiteral = tr("%1 hours later than local").arg(QString::number(-timeDelta, 'f', decimalNumber));
    }

    thisZoneTime = QDateTime::currentDateTime().toTimeZone(thisZone);
    if (hour_24) {
        time = thisZoneTime.toString("hh : mm : ss");
    } else {
        time = thisZoneTime.toString("AP hh: mm : ss");
    }

    labelTime->setText(QString("%1  %2     %3").arg(dateLiteral).arg(time).arg(compareLiteral));
}
