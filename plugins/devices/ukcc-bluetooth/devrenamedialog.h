#ifndef DEVRENAMEDIALOG_H
#define DEVRENAMEDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QLineEdit>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QGSettings>
#include <QVariant>
#include <QPalette>
#include <QKeyEvent>

class DevRenameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DevRenameDialog(QWidget *parent = nullptr);

    ~DevRenameDialog();

    void setDevName(const QString &);

    void initGsettings();

private slots:
    void gsettingsSlot(const QString &);

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);

public slots:
    void lineEditSlot(const QString &);

signals:
    void nameChanged(QString);

private:
    void initUI();

    int  _fontSize;

    bool isblack = false;

    QString adapterOldName;

    QLabel *tipLabel   = nullptr;

    QGSettings *gsettings  = nullptr;

    QPushButton *closeBtn  = nullptr;

    QPushButton *acceptBtn = nullptr;

    QPushButton *rejectBtn = nullptr;

    QLineEdit   *lineEdit  = nullptr;
};

#endif // DEVRENAMEDIALOG_H
