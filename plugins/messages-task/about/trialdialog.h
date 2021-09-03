#ifndef TRIALDIALOG_H
#define TRIALDIALOG_H

#include <QDialog>
#include <QTextBrowser>
#include "Label/titlelabel.h"

class TrialDialog : public QDialog
{
    Q_OBJECT
public:
    TrialDialog(QWidget *parent);
    ~TrialDialog();

private:
    void initUi(QDialog *mTrialDialog);

private:
    TitleLabel *mTitleLabel;

    QLabel *mContentLabel_1;
    QTextBrowser *mContentLabel_2;
    QLabel *mContentLabel_3;
    QLabel *mContentLabel_4;
    QLabel *mContentLabel_5;


};

#endif // TRIALDIALOG_H
