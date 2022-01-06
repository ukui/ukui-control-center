#include "intelcustomizenamelabel.h"
#include "ImageUtil/imageutil.h"


#define STYLE_NAME "org.ukui.style"

CustomizeNameLabel::CustomizeNameLabel(QWidget *parent) : QWidget(parent)
{
    initMemberVariables();
    update();
}

CustomizeNameLabel::~CustomizeNameLabel()
{

}

void CustomizeNameLabel::initMemberVariables()
{
    if (QGSettings::isSchemaInstalled(STYLE_NAME)) {
        gsettings = new QGSettings(STYLE_NAME);
        connect(gsettings,&QGSettings::changed,this,&CustomizeNameLabel::gsettingsChanged);
        if(gsettings->get("style-name").toString() == "ukui-black" ||
           gsettings->get("style-name").toString() == "ukui-dark")
        {
            _themeIsBlack = true;
        }
        else
        {
            _themeIsBlack = false;
        }


        _fontFamily = gsettings->get("system-font").toString();
        _fontSize = gsettings->get("system-font-size").toString().toInt();

    }

    nameLineEdit = new QLineEdit(this);
    nameLineEdit->setEchoMode(QLineEdit::Normal);
    nameLineEdit->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    connect(nameLineEdit,&QLineEdit::editingFinished,this,&CustomizeNameLabel::LineEditFinishSlot);
    connect(nameLineEdit,&QLineEdit::textChanged,this,&CustomizeNameLabel::nameLineEditSlot);
    nameLineEdit->setFixedSize(420,40);
    nameLineEdit->setVisible(false);
    if (_themeIsBlack)
        nameLineEdit->setStyleSheet("QLineEdit {\
                                 border: 1px solid transparent; /* 边框宽度 */\
                                 selection-background-color: #2FB3E8; /* 选中文本的背景颜色 */\
                                 selection-color: #FFFFFF; /* 选中文本的颜色 */\
                                 color: white;\
                                 background-color: transparent; /* 背景颜色 */}");
     else
        nameLineEdit->setStyleSheet("QLineEdit {\
                                 border: 1px solid transparent; /* 边框宽度 */\
                                 selection-background-color: #2FB3E8; /* 选中文本的背景颜色 */\
                                 selection-color: #FFFFFF; /* 选中文本的颜色 */\
                                 background-color: transparent; /* 背景颜色 */}");
}

void CustomizeNameLabel::setAdapterNameText(QString str)
{
    if (str.isEmpty() || str.isNull())
        _adapterName = "";
    else
        _adapterName = str;

    update();
}

void CustomizeNameLabel::enterEvent(QEvent *e)
{
    Q_UNUSED(e);

    _hoverFlag = true;
    update();
}

void CustomizeNameLabel::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);

    _hoverFlag = false;
    update();
}

void CustomizeNameLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);

    qDebug() <<Q_FUNC_INFO <<this->rect();

    _doubleClicked = true;
    update();

    nameLineEdit->setVisible(true);
    nameLineEdit->setText(_adapterName);
    nameLineEdit->selectAll();
    nameLineEdit->setFocus();
}

void CustomizeNameLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

//    painter.setBrush(QColor(Qt::blue));
//    painter.drawRoundedRect(this->rect(),10,10);

    if (!_doubleClicked) {
        DrawText(painter);
        DrawIcon(painter);
    }
}

QColor CustomizeNameLabel::getTextAndIconColor()
{
    if (_hoverFlag) {
        return QColor("#2FB3E8");
    } else {
        if(_themeIsBlack) {
            return QColor(Qt::white);
        } else {
            return QColor(Qt::black);
        }
    }
}

void CustomizeNameLabel::DrawText(QPainter &painter)
{
    QRect rect = getFontPixelQPoint(_adapterName);

    painter.save();

    painter.setPen(getTextAndIconColor());
    painter.drawText(QRect(0,9,rect.width()+5,rect.height()),Qt::AlignLeft,_adapterName);

    painter.restore();
}

void CustomizeNameLabel::DrawIcon(QPainter &painter)
{
    painter.save();

    if (_hoverFlag)
        painter.drawPixmap(QRect(getFontPixelQPoint(_adapterName).width()+10,11,20,20),ImageUtil::loadSvg(":/img/plugins/bluetooth/edit-blue.svg","default",20));
    else
        painter.drawPixmap(QRect(getFontPixelQPoint(_adapterName).width()+10,11,20,20),ImageUtil::loadSvg(":/img/plugins/bluetooth/edit.svg","default",20));

    painter.restore();
}

QRect CustomizeNameLabel::getFontPixelQPoint(QString str)
{
    QFont font;
    font.setFamily(_fontFamily);
    font.setPointSize(_fontSize);
    QFontMetrics fm(font);

    return fm.boundingRect(str);
}

void CustomizeNameLabel::gsettingsChanged(const QString &key)
{
    if(key == "styleName"){
        if(gsettings->get("style-name").toString() == "ukui-black" ||
           gsettings->get("style-name").toString() == "ukui-dark")
        {
            _themeIsBlack = true;
            nameLineEdit->setStyleSheet("QLineEdit {\
                                         border: 1px solid transparent; /* 边框宽度 */\
                                         selection-background-color: #2FB3E8; /* 选中文本的背景颜色 */\
                                         selection-color: #FFFFFF; /* 选中文本的颜色 */\
                                         color: white;\
                                         background-color: transparent; /* 背景颜色 */}");
        }
        else
        {
            _themeIsBlack = false;
            nameLineEdit->setStyleSheet("QLineEdit {\
                                        border: 1px solid transparent; /* 边框宽度 */\
                                        selection-background-color: #2FB3E8; /* 选中文本的背景颜色 */\
                                        selection-color: #FFFFFF; /* 选中文本的颜色 */\
                                        background-color: transparent; /* 背景颜色 */}");

        }
    } else if (key == "systemFont") {
        _fontFamily = gsettings->get("system-font").toString();
    } else if (key == "systemFontSize") {
        _fontSize = gsettings->get("system-font-size").toString().toInt();
    }
}

void CustomizeNameLabel::nameLineEditSlot(const QString &text)
{
    if (text.length() == 0) {
        emit setTipText(1);
        return;
    }
    if (text.length() > 30) {
        emit setTipText(2);
        return;
    }
    emit setTipText(3);
}

void CustomizeNameLabel::LineEditFinishSlot()
{
    if (nameLineEdit->text().length() > 0 &&
        nameLineEdit->text().length() < 30) {

        if (nameLineEdit->text() != _adapterName) {
            _adapterName = nameLineEdit->text();
            emit sendAdapterName(_adapterName);
        }

    }

    nameLineEdit->setVisible(false);
    _doubleClicked = false;
    update();
    emit setTipText(3);
}
