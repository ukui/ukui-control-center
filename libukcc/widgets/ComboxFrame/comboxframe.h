#ifndef COMBOXFRAME_H
#define COMBOXFRAME_H

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>

#include "libukcc_global.h"

class LIBUKCC_EXPORT ComboxFrame : public QFrame
{
    Q_OBJECT
public:
    ComboxFrame(QString labelStr, QWidget *parent = nullptr);
    ComboxFrame(bool isNum, QString labelStr, QWidget *parent = nullptr);
    ~ComboxFrame();

public:
    QComboBox   *mCombox;
    QComboBox   *mNumCombox;
    QLabel      *mTitleLabel;
    QHBoxLayout *mHLayout;

private:
    QString     mTitleName;

signals:
};

#endif // COMBOXFRAME_H
