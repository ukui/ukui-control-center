#ifndef DELETEUSEREXISTS_H
#define DELETEUSEREXISTS_H

#include <QDialog>
#include "../../../shell/utils/utils.h"

class QHBoxLayout;
class QVBoxLayout;
class QFrame;
class QRadioButton;
class QLabel;
class QPushButton;
class QButtonGroup;

class DeleteUserExists : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteUserExists(QString name, QString nick, qint64 uid, QWidget *parent = nullptr);
    ~DeleteUserExists();
public:
    void initUI();
    void setConnect();

public:
    QVBoxLayout * mainVerLayout;
    QHBoxLayout * titleHorLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * noteHorLayout;
    QHBoxLayout * removeButKeepFilesHorLayout;
    QHBoxLayout * removeWholeHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;

    QFrame * removeButKeepFilesFrame;
    QFrame * removeWholeFrame;

    QButtonGroup * removeBtnGroup;

    QRadioButton * removeButKeepFilesRadioBtn;
    QRadioButton * removeWholeRadioBtn;

    QLabel * noteLabel;
    QLabel * removeButKeepFilesLabel;
    QLabel * removeWholeLabel;

    QPushButton * closeBtn;
    QPushButton * cancelBtn;
    QPushButton * confirmBtn;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *);

private:
    QString _name;
    QString _nick;
    qint64 _id;
};

#endif // DELETEUSEREXISTS_H
