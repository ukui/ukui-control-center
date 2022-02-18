#ifndef DEVREMOVEDIALOG_H
#define DEVREMOVEDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QColor>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QGSettings>
#include <QVariant>
#include <QPalette>

class DevRemoveDialog final : public QDialog
{
    Q_OBJECT
public:
    explicit DevRemoveDialog(int mode , QWidget *parent = nullptr);

    ~DevRemoveDialog();

    void initUI();

    void initGsettings();

    void setDialogText(const QString &);

private slots:
    void gsettingsSlot(const QString &);

protected:
    void paintEvent(QPaintEvent *);

private:
    bool isblack = false;

    QLabel *tipLabel  = nullptr;

    QLabel *txtLabel  = nullptr;

    QLabel *iconLabel = nullptr;

    QGSettings *gsettings  = nullptr;

    QPushButton *closeBtn  = nullptr;

    QPushButton *acceptBtn = nullptr;

    QPushButton *rejectBtn = nullptr;

    int m_mode = 1;
};

#endif // DEVREMOVEDIALOG_H
