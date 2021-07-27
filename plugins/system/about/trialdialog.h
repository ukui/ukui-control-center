#ifndef TRIALDIALOG_H
#define TRIALDIALOG_H

#include <QDialog>

class TrialDialog : public QDialog
{
    Q_OBJECT
public:
    TrialDialog(QWidget *parent);
    ~TrialDialog();

private:
    void initUi(QDialog *mTrialDialog);
};

#endif // TRIALDIALOG_H
