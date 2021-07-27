#include "trialdialog.h"

#include <QDebug>

TrialDialog::TrialDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set"));
    initUi(this);
}

TrialDialog::~TrialDialog()
{

}

void TrialDialog::initUi(QDialog *mTrialDialog)
{
    mTrialDialog->resize(300,200);
}
