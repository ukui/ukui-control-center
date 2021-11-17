#ifndef INFOBUTTON_H
#define INFOBUTTON_H
#include <QPushButton>
#include <QIcon>
#include <QPainterPath>

#include "libukcc_global.h"

class LIBUKCC_EXPORT InfoButton : public QPushButton
{
    Q_OBJECT
public:
    explicit InfoButton(QWidget * parent = nullptr);
    ~InfoButton() = default;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void initUI();

private:
    QColor m_backgroundColor;
    QColor m_foregroundColor;
};

#endif // INFOBUTTON_H
