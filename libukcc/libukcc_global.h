#ifndef LIBUKCC_GLOBAL_H
#define LIBUKCC_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBUKCC_LIBRARY)
#  define LIBUKCC_EXPORT Q_DECL_EXPORT
#else
#  define LIBUKCC_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBUKCC_GLOBAL_H
