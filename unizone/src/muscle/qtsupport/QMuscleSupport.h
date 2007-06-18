/* This file is Copyright 2007 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef QMuscleSupport_h
#define QMuscleSupport_h

#include <qstring.h>
#include <qhash.h>

#ifndef MUSCLE_AVOID_NAMESPACES
namespace muscle {
#endif

template <class T> class HashFunctor;

// Enables the use of QStrings as keys in a MUSCLE Hashtable.
template <>
class HashFunctor<QString>
{
public:
   uint32 operator () (const QString & x) const {return qHash(x);}
};

#ifndef MUSCLE_AVOID_NAMESPACES
};  // end namespace muscle
#endif

#endif
