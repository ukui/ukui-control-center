#ifndef RMENU_H
#define RMENU_H

#include <QObject>
#include <QMenu>

class RMenu : public QMenu
{
    Q_OBJECT
public:
    RMenu(QWidget *parent);

private:
    void showEvent(QShowEvent *event);
};

#endif // RMENU_H
