#include "doubleclicklineedit.h"
#include <QDebug>

DoubleClickLineEdit::DoubleClickLineEdit(QList<KeyEntry *> customEntries,QWidget *parent):
    QLineEdit(parent),
    customEntry(customEntries){
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");
    this->setReadOnly(true);
    this->setContextMenuPolicy(Qt::NoContextMenu);
    defaultQss = this->styleSheet();


    connect(this, &DoubleClickLineEdit::textChanged, this, [=](){
        changeName();
        if (false == strIsAvailable) {
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        } else {
           this->setStyleSheet(defaultQss);
        }
    });
}

DoubleClickLineEdit::~DoubleClickLineEdit() {

}

void DoubleClickLineEdit::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    this->setReadOnly(false);
    return;
}

void DoubleClickLineEdit::focusOutEvent(QFocusEvent *e) {
    Q_UNUSED(e);
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");
    this->setReadOnly(true);

    QString str;
    if (strIsAvailable) {
        str = this->text();
    } else {
        str = validStr;
    }
    this->blockSignals(true);
    this->setText(str);
    this->blockSignals(false);
    if (strIsAvailable) {
        Q_EMIT strChanged();
    }
    this->setStyleSheet(defaultQss);
}

void DoubleClickLineEdit::setText(const QString &str) {
    validStr = str;
    QLineEdit::setText(str);
}

void DoubleClickLineEdit::changeName() {
    QStringList customName;
    QString text = this->text();
    if (text.isEmpty()) {
         strIsAvailable = false;
         return;
    }
    if (customEntry.isEmpty()) {
        strIsAvailable = true;
    } else {
        for (KeyEntry *ckeyEntry : customEntry) {
            customName << ckeyEntry->nameStr;
            if (customName.contains(text) && text != validStr) {
                strIsAvailable = false;
            } else {
                strIsAvailable = true;
            }
        }
    }
}

void DoubleClickLineEdit::updateCustomEntry(QList<KeyEntry *> customEntries) {
    this->customEntry = customEntries;
}

/*-----------------------------------------DoubleClickShortCut-----------------------------------------------*/

DoubleClickShortCut::DoubleClickShortCut(QList<KeyEntry *> generalEntries, QList<KeyEntry *> customEntries,
                                         QWidget *parent) :
    ShortcutLine(generalEntries,customEntries,parent){
    this->customEntry = customEntries;
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");
    this->setReadOnly(true);
    this->setContextMenuPolicy(Qt::NoContextMenu);
    defaultQss = this->styleSheet();

    connect(this, &DoubleClickShortCut::textChanged, this, [=](){
        if (this->text().isEmpty()) {
            shortcutIsAvailable = false;
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        }
    });

    connect(this, &DoubleClickShortCut::shortCutAvailable, this, [=](int flag) {
        if (0 == flag) {
            shortcutIsAvailable = true;
            this->setStyleSheet(defaultQss);
        } else {
            shortcutIsAvailable = false;
            this->setStyleSheet("border:2px solid red;border-radius:6px;padding:3px 4px");
        }
    });
}

DoubleClickShortCut::~DoubleClickShortCut() {

}

void DoubleClickShortCut::mouseDoubleClickEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    this->setReadOnly(false);
    return;
}

void DoubleClickShortCut::focusOutEvent(QFocusEvent *e) {
    //this->setStyleSheet("QLineEdit {background-color : palette(base);}");
    this->setReadOnly(true);

    QString str;
    if (shortcutIsAvailable) {
        str = this->text();
    } else {
        str = validShortcut;
    }
    this->blockSignals(true);
    this->setText(str);
    this->blockSignals(false);
    if (shortcutIsAvailable) {
        Q_EMIT shortcutChanged();
    }
    this->setStyleSheet(defaultQss);
    ShortcutLine::focusOutEvent(e);
}

void DoubleClickShortCut::setText(const QString &str) {
    validShortcut = str;
    QLineEdit::setText(str);
}
