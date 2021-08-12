#include "mylayout.h"

MyLayout::MyLayout(QWidget *parent)
{
}
QLayoutItem *MyLayout::itemAt(int i) const
{
    return list.value(i);
}  //返回索引i处的项目。
QLayoutItem *MyLayout::takeAt(int i)   		//删除索引i处的项目
{
    return i >= 0 && i < list.size() ? (QLayoutItem*)list.takeAt(i) : 0;
}
int MyLayout::count() const{
    return list.size();
}  	//返回布局中的项目数量
MyLayout::~MyLayout()
{
    //因为QLayoutItem未继承自QObject，因此必须手动删除QLayoutItem对象。
       QLayoutItem *item;
       while ((item = takeAt(0)))
           delete item;
}
void MyLayout::setGeometry(const QRect &r)
{		//布置布局中的子项目
      QSize s=parentWidget()->size();
      //获取布局所在父部件的大小
      int w=sizeHint().width();
      int h=sizeHint().height();
      int x=0; int y=0;
      //部件左上角的坐标。
      for(int i=0;i<list.size();i++){
          list.at(i)->setGeometry(QRect(x,y,w,h));
          x=x+w;
        //第二个项目的水平坐标向后移第一个部件的宽度
        if(x+w>s.width())/*如果新添加的项目占据的位置超过了父部件的大小，则该部件添加到下一行的开头*/
        {
            y=y+h;
            x=0;
        }
      }
}
QSize MyLayout::sizeHint() const
{
    return QSize(276,312);
}
void MyLayout::addItem(QLayoutItem *item)
{
    list.append(item);
}  //把元素添加到列表。
void MyLayout::addw(QWidget* p)
{

    addItem(new QWidgetItem(p)); //把p转换为QLayoutItem对象，非QLayoutItem对象不能由布局管理。
    //addItem((QLayoutItem*)p);  /*错误，强制类型转换指针的类型，会使内存的内容被重新解释，这可能会产生内存错误。比如int a=1; int *p=&a; 假设int占4字节，double占8字节，则*p只会读取4字节的内容，但是*(double*)p;则会读取8字节的内容(详见《C++语法详解》一书有关指针的讲解)。*/
}
