#ifndef TOOLPOP_H
#define TOOLPOP_H

#include <QLabel>

class ToolPop : public QLabel
{
    Q_OBJECT
public:
    explicit ToolPop(QWidget* parent = nullptr);

public slots:
    // 弹出时区位置
    void popupSlot(QPoint ponit);

protected:
    void paintEvent(QPaintEvent* event);
};

#endif // TOOLPOP_H
