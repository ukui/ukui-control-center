#ifndef CHANGEFEATURENAME_H
#define CHANGEFEATURENAME_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>

namespace Ui {
class ChangeFeatureName;
}

class ChangeFeatureName : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeFeatureName(QStringList names,QWidget *parent = nullptr);
    ~ChangeFeatureName();

private:
    QStringList names;

private:
    Ui::ChangeFeatureName *ui;

protected:
    void paintEvent(QPaintEvent *);

Q_SIGNALS:
    void sendNewName(QString name);
};

#endif // CHANGEFEATURENAME_H
