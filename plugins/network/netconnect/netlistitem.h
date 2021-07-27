#ifndef NETLISTITEM_H
#define NETLISTITEM_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class NetListItem : public QWidget
{
    Q_OBJECT
public:
    explicit NetListItem(const QString &icon_path, const QString &net_name, QWidget *parent = nullptr);
    ~NetListItem();

    void setActive(const bool &isAct);
    bool isActive();
    void initWifiWidget(const QString &icon_path, const QString &net_name);
    void initLanWidget(const QString &icon_path, const QString &net_name);

private:
    QVBoxLayout * m_baseVerLayout = nullptr;
    QFrame * m_devFrame = nullptr;
    QHBoxLayout * m_devHorLayout = nullptr;
    QLabel * m_iconLabel = nullptr;
    QLabel * m_nameLabel = nullptr;
    QLabel * m_statusLabel = nullptr;
    QLabel * m_circleLabel = nullptr;
    int m_signal = 1;
    bool m_secu = true;
    QString m_name = 0;
    bool m_active = false;
signals:

};

#endif // NETLISTITEM_H
