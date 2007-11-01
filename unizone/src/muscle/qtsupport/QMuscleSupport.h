/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef QMuscleSupport_h
#define QMuscleSupport_h

#if QT_VERSION >= 0x040000
# include <qhash.h>
#else
# include "util/String.h"
#endif

#include "support/MuscleSupport.h"  // for uint32, etc

#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
# include "util/Hashtable.h"
#endif

#ifndef MUSCLE_AVOID_NAMESPACES
namespace muscle {
#endif

#if (!defined(_MSC_VER)) || (_MSC_VER >= 1300)
template <class T> class HashFunctor;
#endif

// Enables the use of QStrings as keys in a MUSCLE Hashtable.
template <>
class HashFunctor<QString>
{
public:
#if QT_VERSION >= 0x040000
   uint32 operator () (const QString & x) const {return qHash(x);}
#else
   uint32 operator () (const QString & x) const {return CStringHashFunc(x.utf8().data());}
#endif
};

#ifndef MUSCLE_AVOID_NAMESPACES
};  // end namespace muscle
#endif

#endif
