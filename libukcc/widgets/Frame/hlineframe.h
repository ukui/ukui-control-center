#ifndef _HLINEFRAME_H_
#define _HLINEFRAME_H_
#include <QFrame>

#include "libukcc_global.h"

class LIBUKCC_EXPORT HLineFrame : public QFrame
{
public:
    HLineFrame(QWidget *parent = nullptr);
    ~HLineFrame();
};



#endif
