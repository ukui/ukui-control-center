#ifndef ICONTHEMEWIDGET_H
#define ICONTHEMEWIDGET_H

//#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QMouseEvent>


class IconThemeWidget : public QFrame
{
    Q_OBJECT

public:
    explicit IconThemeWidget();
    ~IconThemeWidget();

public:
    void set_icontheme_name(QString name);
    void set_icontheme_selected(bool status);
    void set_icontheme_example(QStringList firsticonList, QStringList secondiconList);
    void set_icontheme_fullname(QString fullname);
    QString get_icontheme_fullname();

protected:
    void mousePressEvent(QMouseEvent * event);

private:
    QHBoxLayout * mainHBoxLayout;
    QHBoxLayout * flHBoxLayout;
    QHBoxLayout * slHBoxLayout;
    QVBoxLayout * leftVBoxLayout;
    QVBoxLayout * rightVBoxLayout;

    QLabel * themeName;
    QLabel * themeStatus;
    QLabel * selectedIcon;

    QString themeFullName;

Q_SIGNALS:
    void clicked(QString fullname);

};

#endif // ICONTHEMEWIDGET_H
