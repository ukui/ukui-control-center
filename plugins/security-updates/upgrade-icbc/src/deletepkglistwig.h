#ifndef DELETEPKGLISTWIG_H
#define DELETEPKGLISTWIG_H


#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QGSettings>
#include <QDebug>

class DeletePkgListWig : public QFrame
{
    Q_OBJECT
public:
    DeletePkgListWig(QLabel *);
    ~DeletePkgListWig();
    void setAttribute(QString &mname, QString &mdescription,QString &deletereason);//赋值
    QSize getTrueSize();//获取真实大小
    void selectStyle();//选中样式
    void clearStyleSheet();//取消选中样式
    int id = 0;
    void gsettingInit();
protected:
    void mousePressEvent(QMouseEvent * event);
private:
    QHBoxLayout *hl1 = nullptr;
    QVBoxLayout *vl1 = nullptr;
    QLabel *debName = nullptr;//app名字&版本号
    QString debDescription = "";//描述
    QString debdeletereason = "";//删除原因
    //int code = 0 ;//编码
    void initUI();//初始化UI
    void setDescription();//赋值事件
    QFont font;

    /* 监听主题字体变化 */
    QGSettings *m_pGsettingFontSize;
    QString mNameLabel;
    QString mStatusLabel;

    /* 控制悬浮框 */
    bool nameTipStatus = false;
    bool statusTipStatus = false;

    /* 右侧顶部标签 */
    QLabel *mDesTab = nullptr;
    QLabel *deleteReasonTab = nullptr;
};
#endif // DELETEPKGLISTWIG_H
