#ifndef DOUBLECLICKLINEEDIT_H
#define DOUBLECLICKLINEEDIT_H
#include <QLineEdit>
#include "keyentry.h"
#include "shortcutline.h"
#include <QStyleOption>
#include <QPainter>

class DoubleClickLineEdit : public QLineEdit {
    Q_OBJECT
public:
    DoubleClickLineEdit(QList<KeyEntry *> *customEntries, QWidget *parent = nullptr);
    ~DoubleClickLineEdit();
    void setText(const QString &str);
    void changeName();
protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent *e);

private:
    QString validStr;
    bool strIsAvailable;
    QList<KeyEntry *> *customEntry;
    QString defaultQss;

Q_SIGNALS:
    void focusOut();
    void strChanged();
};



class DoubleClickShortCut : public ShortcutLine {
    Q_OBJECT
public:
    DoubleClickShortCut(QList<KeyEntry *> generalEntries, QList<KeyEntry *> *customEntries,
                        QWidget *parent = nullptr);
    ~DoubleClickShortCut();
    void setText(const QString &str);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent *e);

private:
    QString validShortcut;
    bool shortcutIsAvailable;
    QList<KeyEntry *> *customEntry;
    QString defaultQss;

Q_SIGNALS:
    void focusOut();
    void shortcutChanged();
};


#endif // DOUBLECLICKLINEEDIT_H
