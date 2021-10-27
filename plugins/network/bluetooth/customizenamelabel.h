#ifndef CUSTOMIZENAMELABEL_H
#define CUSTOMIZENAMELABEL_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QFontMetrics>
#include <QGSettings>
#include <QString>
#include <QLineEdit>
#include <QPixmap>
#include <QColor>
#include <QDebug>

class CustomizeNameLabel : public QWidget
{
    Q_OBJECT
public:
    explicit CustomizeNameLabel(QWidget *parent = nullptr);
    ~CustomizeNameLabel();

    void initMemberVariables();

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

    void DrawText(QPainter &);
    void DrawIcon(QPainter &);

    QColor getTextAndIconColor();
    QRect  getFontPixelQPoint(QString);

public slots:
    void setAdapterNameText(QString);

protected slots:
    void gsettingsChanged(const QString &);
    void nameLineEditSlot(const QString &text);
    void LineEditFinishSlot();

signals:
    void setTipText(int);
    void sendAdapterName(const QString &value);

private:
    QGSettings *gsettings    = nullptr;
    QLineEdit  *nameLineEdit = nullptr;

    QString _fontFamily;
    QString _adapterName = "";

    int _fontSize = 0;

    bool _hoverFlag = false;
    bool _themeIsBlack  = false;
    bool _doubleClicked = false;
};

#endif // CUSTOMIZENAMELABEL_H
