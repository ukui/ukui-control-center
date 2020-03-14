#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QTimer>
#include <QDialog>
#include <QGSettings/QGSettings>

namespace Ui {
class DataFormat;
}

class DataFormat : public QDialog
{
    Q_OBJECT

public:
    explicit DataFormat(QWidget *parent = nullptr);
    ~DataFormat();

private:
    QTimer *m_itimer;
    QGSettings *m_gsettings = nullptr;

private:
    Ui::DataFormat *ui;

    void initUi();
    void initConnect();
    void initComponent();

private slots:
    void datetime_update_slot();
    void calendar_change_slot(int);
    void day_change_slot(int);
    void date_change_slot(int);
    void time_change_slot(int);

};

#endif // DATAFORMAT_H
