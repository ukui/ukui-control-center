#ifndef APPLYDIALOG_H
#define APPLYDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QSharedPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

namespace Ui {
class ApplyDialog;
}

class ApplyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyDialog(QWidget *parent = nullptr);
    ~ApplyDialog();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::ApplyDialog *ui;

    void initUI();
    //总布局
    QSharedPointer<QVBoxLayout> mainLyt;
    //标题栏
    QSharedPointer<QFrame> titleFrame;
    QSharedPointer<QHBoxLayout> titleLyt;
    QPushButton * minimizeBtn = nullptr;
    QPushButton * closeBtn = nullptr;
    //分割线
    QSharedPointer<QFrame> hLine;
    //内容控件
    QSharedPointer<QFrame> contentFrame;
    QSharedPointer<QVBoxLayout> contentLyt;
    QSharedPointer<QLabel> tipLabel;
    QSharedPointer<QLabel> countDownLabel;
    //按钮
    QSharedPointer<QFrame> buttonFrame;
    QSharedPointer<QHBoxLayout> buttonLyt;
    QPushButton * cancelBtn = nullptr;
    QPushButton * applyBtn = nullptr;

    void initTimer(int second); //计时器
    int sec;
    QTimer * m_itimer = nullptr;

Q_SIGNALS:
    void do_not_apply();
    void do_apply();

private slots:
    void updateLabel();
};

#endif // APPLYDIALOG_H
