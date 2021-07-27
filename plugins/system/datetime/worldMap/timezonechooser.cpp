#include "timezonechooser.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QTimeZone>
#include <QCompleter>
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDir>
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
TimeZoneChooser::TimeZoneChooser():QFrame ()
{
    m_map = new TimezoneMap(this);
    m_map->show();
    m_zoneinfo = new ZoneInfo;
    m_searchInput = new QLineEdit;
    m_title = new QLabel;
    closeBtn = new QPushButton;
    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_confirmBtn = new QPushButton(tr("Confirm"));

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);//无边框
    setAttribute(Qt::WA_StyledBackground,true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    this->setObjectName("MapFrame");
//    this->setStyleSheet("QFrame#MapFrame{background-color: rgb(22, 24, 26);border-radius:4px}");
    QLabel * titleLabel = new QLabel;
    titleLabel->setText(tr("Change the time zone"));
    titleLabel->setStyleSheet("font-size: 16px;"
                              "font-weight: 400;");
//    closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"default"));
    closeBtn->setFlat(true);
//    closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
//                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
    timeZoneBox = new QComboBox;
    timeZoneBox->setFixedSize(m_map->width()-14,48);
    int count = 0;
    for (QString timezone : QTimeZone::availableTimeZoneIds()) {
        count ++;
        const QString locale = QLocale::system().name();
        QString localizedTimezone = m_zoneinfo->getLocalTimezoneName(timezone, locale);
        if (locale == "zh_CN") {
            if (localizedTimezone == "Nuuk") {
                localizedTimezone = "努克";
            }
            if (localizedTimezone == "Qostanay") {
                localizedTimezone = "哈萨克斯坦";
            }
        }
        if (!localizedTimezone.startsWith("UTC")) {
            timeZoneBox->addItem(localizedTimezone,count);
        }
    }
    QString preZone = m_zoneinfo->getCurrentTimzone();
    const QString locale = QLocale::system().name();
    QString localizedTimezone = m_zoneinfo->getLocalTimezoneName(preZone, locale);
    timeZoneBox->setCurrentText(localizedTimezone);

    m_searchInput->setMinimumSize(560,40);
    m_searchInput->setMaximumSize(560,40);
    m_searchInput->setMinimumHeight(40);
    m_searchInput->setStyleSheet("QLineEdit{background: palette(button); border: none; border-radius: 8px; font-size: 14px; color: palette(text);}");
//    m_searchInput->setStyleSheet("background-color: rgb(229, 240, 250 )");
//    m_cancelBtn->setStyleSheet("background-color: rgb(229, 240, 250 )");
    m_confirmBtn->setStyleSheet("background-color: rgb(47, 179, 232)"); //2FB3E8

/*    m_title->setMinimumWidth(179);
    m_title->setMinimumHeight(29);*/;
    m_title->setObjectName("titleLabel");
    m_title->setStyleSheet("color: rgb(229, 240, 250 )");
    m_title->setText(tr("change timezone"));


    initSize();

    QHBoxLayout *wbLayout = new QHBoxLayout;
    wbLayout->setMargin(16);
    wbLayout->setSpacing(0);
    wbLayout->addItem(new QSpacerItem(9,10,QSizePolicy::Fixed));
    wbLayout->addWidget(titleLabel);
    wbLayout->addStretch();
    wbLayout->addWidget(closeBtn);

    QHBoxLayout *btnlayout = new QHBoxLayout;
    btnlayout->addStretch();
    btnlayout->addWidget(m_cancelBtn);
    btnlayout->addSpacing(20);
    btnlayout->addWidget(m_confirmBtn);
    btnlayout->addItem(new QSpacerItem(30,10,QSizePolicy::Fixed));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addLayout(wbLayout);
    layout->addStretch();
    layout->addItem(new QSpacerItem(16,24,QSizePolicy::Fixed));
    layout->addWidget(timeZoneBox, 0, Qt::AlignHCenter);
    layout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    layout->addWidget(m_map, 0, Qt::AlignHCenter);
    layout->addItem(new QSpacerItem(16,24,QSizePolicy::Fixed));
    layout->addLayout(btnlayout);
    layout->addItem(new QSpacerItem(16,24,QSizePolicy::Fixed));
    layout->addStretch();

    setLayout(layout);

    connect(m_confirmBtn, &QPushButton::clicked,[this]{
        QString timezone = m_map->getTimezone();
        emit this->confirmed(timezone);
    });

    connect(m_cancelBtn, &QPushButton::clicked, this, [this]{
        hide();
        emit this->cancelled();
    });

    connect(closeBtn, &QPushButton::clicked, this, [this] {
        hide();
        emit cancelled();
    });


    connect(m_map, &TimezoneMap::timezoneSelected, this, [this]{
        m_searchInput->setText("") ;
        m_searchInput->clearFocus();
    });

    connect(m_searchInput, &QLineEdit::editingFinished, [this]{
        QString timezone = m_searchInput->text();
        timezone = m_zoneCompletion.value(timezone,timezone);
        m_map->setTimezone(timezone);
    });


    QTimer::singleShot(0, [this] {

//        qDebug()<<"single slot-------->"<<endl;
        QStringList completions;
        for (QString timezone : QTimeZone::availableTimeZoneIds()) {
            completions << timezone;

            const QString locale = QLocale::system().name();
            QString localizedTimezone = m_zoneinfo->getLocalTimezoneName(timezone, locale);
            completions << localizedTimezone;
            m_zoneCompletion[localizedTimezone] = timezone;
        }

        QCompleter *completer = new QCompleter(completions, m_searchInput);
        completer->popup()->setAttribute(Qt::WA_InputMethodEnabled);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setCaseSensitivity(Qt::CaseInsensitive);

        m_searchInput->setCompleter(completer);

        m_popup = completer->popup();
        m_popup->setAttribute(Qt::WA_TranslucentBackground);
        m_popup->installEventFilter(this);

        QHBoxLayout *layout = new QHBoxLayout;
        layout->setSpacing(0);
        layout->setMargin(0);
        m_popup->setLayout(layout);
    });
    connect(timeZoneBox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[=]{
        QString currentZone = timeZoneBox->currentText();
        currentZone = m_zoneCompletion.value(currentZone,currentZone);
        m_map->setTimezone(currentZone);
    });
}
const QPixmap TimeZoneChooser::renderSvg(const QIcon &icon, QString cgColor) {
    int size = 24;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 48;
    } else if (3 == ratio) {
        size = 96;
    }
    QPixmap iconPixmap = icon.pixmap(size,size);
    iconPixmap.setDevicePixelRatio(ratio);
    QImage img = iconPixmap.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return iconPixmap;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}
void TimeZoneChooser::setTitle() {
    m_title->setText(tr("change timezone"));;
}

void TimeZoneChooser::setMarkedTimeZoneSlot(QString timezone) {
    m_map->setTimezone(timezone);
}

void TimeZoneChooser::keyRealeaseEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Cancel)) {
        hide();
        emit this->cancelled();
    }
}

bool TimeZoneChooser::eventFilter(QObject* obj, QEvent *event) {
    if (obj == m_popup && event->type() == QEvent::Move) {
        QMoveEvent* move = static_cast<QMoveEvent*>(event);
        QPoint desPos = m_searchInput->mapToGlobal(QPoint(0, m_searchInput->height() + 2));

        if(move->pos() != desPos) {
            m_popup->move(desPos);
        }
    }
    return false;
}

void TimeZoneChooser::paintEvent(QPaintEvent *e)
{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setBrush(QBrush(QColor(22, 24, 26)));
//    p.setPen(QColor(22, 24, 26));
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.drawRoundedRect(opt.rect,6,6);
//    QPainterPath path;
////    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 16, 16);

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
    p.restore();

}
//获取适合屏幕的地图大小
QSize TimeZoneChooser::getFitSize(){
    const QDesktopWidget *desktop = QApplication::desktop();
    const QRect primaryRect = desktop->availableGeometry(desktop->primaryScreen());

    double width = m_map->width() + 48;
    double height = m_map->height() + 224;
//    double width = primaryRect.width() - 360/* dcc */ - 20 * 2;
//    double height = primaryRect.height() - 70/* dock */ - 20 * 2;
//    double width =1440;
//    double height =860;

    return QSize(width,height);
}


void TimeZoneChooser::initSize(){

    double MapPixWidth = 852.0;
    double MapPixHeight = 436.0;
    double MapPictureWidth = 857.0;
    double MapPictureHeight = 436.0;

    QFont font = m_title->font();
    font.setPointSizeF(16.0);
    m_title->setFont(font);

    const QSize fitSize = getFitSize();
    setFixedWidth(fitSize.width());

    const float mapWidth = qMin(MapPixWidth, 1520 - 20 * 2.0);
    const float mapHeight = qMin(MapPixHeight, 924 - 20 * 2/*paddings*/ - 36 * 2/*buttons*/ - 10/*button spacing*/ - 40 * 3.0 /*spacings*/ - 30/*title*/ -  20 * 2/*top bottom margin*/);
    const double widthScale = MapPictureWidth / mapWidth;
    const double heightScale = MapPictureHeight / mapHeight;
    const double scale = qMax(widthScale, heightScale);

//    qDebug()<<"scale------>"<<MapPictureWidth / scale<<" "<<MapPictureHeight / scale<<endl;
    m_map->setFixedSize(MapPictureWidth / scale, MapPictureHeight / scale);

//    m_searchInput->setFixedWidth(250);
    m_cancelBtn->setFixedSize(112,48);
    m_confirmBtn->setFixedSize(112,48);
}
