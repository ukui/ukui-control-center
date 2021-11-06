#include "ksc_module_func_widget.h"
#include "ui_ksc_module_func_widget.h"
#include <QMouseEvent>
#include <QEvent>
#include <QStyle>
#include <QDebug>
#include "fontwatcher.h"
#include <locale.h>
#include <libintl.h>
#define THEME_QT_SCHEMA "org.ukui.style"
#define _(STRING) gettext(STRING)

ksc_module_func_widget::ksc_module_func_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ksc_module_func_widget)
{
    ui->setupUi(this);
    ui->module_name->setObjectName("ksc_module_func_widget_name_label");
    ui->module_name->setProperty("mouse_state", "normal");
    ui->module_name->setText(tr("TextLabel"));
    ui->module_icon->setStyleSheet("background:transparent;");
    ui->module_name->setStyleSheet("background:transparent;");

    QFont font;
    font.setPixelSize(18);
    font.setBold(true);
    ui->module_name->setFont(font);

    QFont ft;
    ft.setPixelSize(15);
    ui->first_label->setFont(ft);
//    ui->frame->setFrameShape(QFrame::Shape::Box);
    ui->second_label->setVisible(false);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    this->setStyleSheet("background:palette(Base);"
                        "border-radius:6px;");
}

ksc_module_func_widget::~ksc_module_func_widget()
{
    delete ui;
}

void ksc_module_func_widget::set_module_data(const ksc_defender_module module)
{
    update_module_data(module);
    ui->module_icon->setPixmap(QPixmap(m_module.module_normal_icon).scaled(ui->module_icon->size(),
                                                                           Qt::IgnoreAspectRatio,
                                                                           Qt::SmoothTransformation));
}

void ksc_module_func_widget::update_module_data(ksc_defender_module module)
{
    m_module.module_type = module.module_type;
    m_module.module_name = module.module_name;
    m_module.module_desc = module.module_desc;
    m_module.module_normal_icon = module.module_normal_icon;
    m_module.module_hover_icon = module.module_hover_icon;
    m_module.module_exec = module.module_exec;
    m_module.status_list = module.status_list;

    ui->first_label->clear();
    ui->second_label->clear();
    if (module.status_list.size() == 1) {
        QString str = _(module.status_list.at(0).toLocal8Bit().data());
        ui->first_label->setText(str);
    }

    if (module.status_list.size() == 2) {
        QString str = QString(_(module.status_list.at(0).toUtf8().data())).arg(module.status_list.at(
                                                                                   1));
        ui->first_label->setText(str);
    }

    QString str_name = _(module.module_name.toLocal8Bit().data());
    ui->module_name->setText(str_name);

     // 添加搜索索引
    //~ contents_path /securitycenter/Security Scan
     tr("Security Scan");
     //~ contents_path /securitycenter/Account Protection
     tr("Account Protection");
     //~ contents_path /securitycenter/Network Protection
     tr("Network Protection");
     //~ contents_path /securitycenter/Application Protection
     tr("Application Protection");

}

void ksc_module_func_widget::update_module_icon()
{
    ui->module_icon->setPixmap(QPixmap(m_module.module_normal_icon).scaled(ui->module_icon->size(),
                                                                           Qt::IgnoreAspectRatio,
                                                                           Qt::SmoothTransformation));
}

void ksc_module_func_widget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    setBackgroundRole(QPalette::Highlight);
    setAutoFillBackground(true);
    this->setStyleSheet("background:palette(Highlight);"
                        "border-radius:6px;");

    ui->module_icon->setPixmap(QPixmap(m_module.module_hover_icon).scaled(ui->module_icon->size(),
                                                                          Qt::IgnoreAspectRatio,
                                                                          Qt::SmoothTransformation));
    QWidget::enterEvent(event);
}

void ksc_module_func_widget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    this->setStyleSheet("background:palette(Base);"
                        "border-radius:6px;");

    ui->module_icon->setPixmap(QPixmap(m_module.module_normal_icon).scaled(ui->module_icon->size(),
                                                                           Qt::IgnoreAspectRatio,
                                                                           Qt::SmoothTransformation));
    QWidget::leaveEvent(event);
}

void ksc_module_func_widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ksc_module_func_widget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button())
        runExternalApp(m_module.module_exec);

    QWidget::mousePressEvent(event);
}
