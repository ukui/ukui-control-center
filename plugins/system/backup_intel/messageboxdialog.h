#ifndef MESSAGEBOXDIALOG_H
#define MESSAGEBOXDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
namespace Ui {
class MessageBoxDialog;
}

class MessageBoxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBoxDialog(QWidget *parent = nullptr);
    ~MessageBoxDialog();

private:
    Ui::MessageBoxDialog *ui;
    void paintEvent(QPaintEvent *event);
    void initUi();
};

#endif // MESSAGEBOXDIALOG_H
