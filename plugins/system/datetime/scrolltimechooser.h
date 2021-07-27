#ifndef SCROLLTIMECHOOSER_H
#define SCROLLTIMECHOOSER_H
#include <QWidget>
#include <QMouseEvent>
#include <QColor>
#include<QPainter>
#include<QTime>
#include <QHBoxLayout>
#include <QPushButton>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

class ScrollTimeChooser : public QWidget
{
    Q_OBJECT
public:
    /**定义类型的目的是滚动屏有垂直和水平之分，为了实现两种模式**/
    enum ScrollType{
      VERTICAL = 1,
      HORIZONTAL
    };
    /**这个只是界面的颜色显示**/
    enum ColorType{
        BACKHROUND = 1,
        LINE,
        CURRENTTEXT,
        DISABLETEXT
    };
public:
    explicit ScrollTimeChooser(QWidget *parent = 0, ScrollType nType = VERTICAL);
    ~ScrollTimeChooser();
public:
    /**
     * @brief      设置界面显示的颜色
     *
     * @param[in]  color		颜色
     * @param[in]  ColorType    颜色的类型,区分颜色的
    **/
    void setColor(QColor &pColor, ColorType nType = CURRENTTEXT);

    /**
     * @brief      获取当前值
     *
     * @return     int
     * @retval     当前在界面中间的值
    **/
    inline int getValue() { return m_nCurrentValue;}

    /**
     * @brief      设置当前值
     *
     * @param[in]  int	当前值
    **/
    inline void setValue(int nValue) { m_nCurrentValue = nValue;}

    /**
     * @brief      设置步长
     *
     * @param[in]  int	步长 界面相邻显示的数值间隔 默认为 1
    **/
    inline void setStep(int nStep) { m_nStep = nStep;}

    /**
     * @brief      设置界面显示的个数
     *
     * @param[in]  int	页面中显示多少个数字
    **/
    inline void setDevice(int nDevice) {m_nDevice = nDevice;}

    /**
     * @brief      设置滚动屏的类型 默认为垂直
     *
     * @param[in]  ScrollType	滚动屏的类型
    **/
    inline void setScrollType(ScrollType nType) {m_nType = nType;}

    /**
     * @brief      设置滚动范围
     *
     * @param[in]  int	最小值
     * @param[in]  int	最大值
    **/
    inline void setRang(int nMin, int nMax)
    {
        m_nMin = nMin;
        m_nMax = nMax;
        m_nCurrentValue = m_nCurrentValue > m_nMax ? m_nMax : m_nCurrentValue;
        m_nCurrentValue = m_nCurrentValue < m_nMin ? m_nMin : m_nCurrentValue;
    }
/**主要是鼠标事件的重实现，其实我们用的最多的应该是滚轮，因此也实现轮子事件**/
protected:
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    /**
     * @brief      绘制背景
     *
     * @param[in]  QPainter	QPainter指针
    **/
    void paintBackground(QPainter* pPainter);

    /**
     * @brief      绘制线条
     *
     * @param[in]  QPainter	QPainter指针
    **/
    void paintLine(QPainter* pPainter);

    /**
     * @brief      绘制字体
     *
     * @param[in]  QPainter	QPainter指针
     * @param[in]  int	需要绘制的数值
     * @param[in]  int	绘制的偏移量 相对于中心便宜多少量绘制数值
     * @param[in]  int	字体的大小
    **/
    void paintText(QPainter* pPainter, int nValue, int nOffSet, int nFontSize);

private:
    int m_nCurrentValue;
    int m_nOffSet;        	//偏离值
    int m_nMax;           	//滚动的最大值
    int m_nMin;           	//滚动的最小值
    int m_nMousePos;		//鼠标点击的位置
    int m_nDevice;      	//显示的数量
    int m_nStep;        	//滚动的步长
    ScrollType  m_nType;    //垂直还是水平
    QColor  m_cBackground;	//背景颜色
    QColor  m_cCurrentText;	//当前值颜色
    QColor  m_cDisableText;	//其他字体颜色
    QColor  m_cLine;		//线条颜色
Q_SIGNALS:
    void signal_currentValueChange();
};

#endif // SCROLLTIMECHOOSER_H
