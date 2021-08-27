#ifndef NAMELABEL_H
#define NAMELABEL_H

#include <QWidget>
#include <QString>
#include <QLabel>

class NameLabel : public QWidget
{
    Q_OBJECT
public:
    explicit NameLabel(QWidget *parent = nullptr);
    void    setText(QString text);

signals:
    void clicked();
 
protected:
    virtual void mouseReleaseEvent(QMouseEvent * ev); 

private:
     QLabel *nameLabel = nullptr;
     QLabel  *changeNameLabel = nullptr;

};

#endif // NAMELABEL_H
