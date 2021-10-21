#include "hlineframe.h"

HLineFrame::HLineFrame(QWidget *parent)
    :QFrame(parent) {

    this->setFrameShape(QFrame::Shape::HLine);
    this->setFixedHeight(1);
}

HLineFrame::~HLineFrame() {

}
