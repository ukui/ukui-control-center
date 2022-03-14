#ifndef MSGBOX_H
#define MSGBOX_H

#include <QObject>
#include <QDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QGSettings>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QPixmap>
#include <QIcon>
#include <QDebug>

class MsgBox : public QDialog
{
    Q_OBJECT
public:
    explicit MsgBox(QWidget *parent = nullptr,const QString = "");
    ~MsgBox();

protected:
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *event);

private slots:
    void gsettingsChanged(const QString &);

private:
    QGSettings  *gsettings = nullptr;
    QPushButton *closeBtn = nullptr;
    QPushButton *okBtn    = nullptr;
    QPushButton *noBtn    = nullptr;

    bool    isBlack = false;
    QString devname;
};

#endif // MSGBOX_H
