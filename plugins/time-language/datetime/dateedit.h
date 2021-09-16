#ifndef DATEEDIT_H
#define DATEEDIT_H
#include <QDateEdit>

class DateEdit : public QDateEdit {
    Q_OBJECT
public :
    DateEdit(QWidget *parent = nullptr);
    ~DateEdit();
    QPixmap loadSvg(const QString &path, int size);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    bool hoverFlag = false;
    bool focusFlag = false;
Q_SIGNALS:
    void changeDate();
};

#endif // DATEEDIT_H
