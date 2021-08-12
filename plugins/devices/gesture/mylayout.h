#ifndef MYLAYOUT_H
#define MYLAYOUT_H

#include <QObject>
#include <QLayout>
#include <QWidget>
class MyLayout : public QLayout
{
    Q_OBJECT
public:
    MyLayout(QWidget *parent);
    ~MyLayout();

//public:      B(QWidget *parent): QLayout(parent) {}     B() {}
//声明需要实现的成员函数
void addItem(QLayoutItem *item);
QSize sizeHint() const;
int count() const;
QLayoutItem *itemAt(int) const;
QLayoutItem *takeAt(int);
void setGeometry(const QRect &rect);
void addw(QWidget* pw);  /*使用一个自定义的函数向布局中添加QWidget对象，addItem函数不能直接接收QWidget对象，该函数主要起类型转换的作用。*/
QList<QLayoutItem*> list;  //使用QList存储布局需要管理的对象。

};

#endif // MYLAYOUT_H

