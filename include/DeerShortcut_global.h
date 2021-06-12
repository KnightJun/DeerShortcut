#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(DeerShortcut_LIB)
#  define DeerShortcut_EXPORT Q_DECL_EXPORT
# else
#  define DeerShortcut_EXPORT Q_DECL_IMPORT
# endif
#else
# define DeerShortcut_EXPORT
#endif
