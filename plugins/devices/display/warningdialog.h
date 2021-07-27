#ifndef WARNINGDIALOG_H
#define WARNINGDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QSharedPointer>

namespace Ui {
class WarningDialog;
}

class WarningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarningDialog(QString warningText, QWidget *parent = nullptr);
    ~WarningDialog();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::WarningDialog *ui;
    void initUI();
    QSharedPointer<QLabel> textLabel;
    QSharedPointer<QLabel> iconLabel;
    QString warning;
};

#endif // WARNINGDIALOG_H
