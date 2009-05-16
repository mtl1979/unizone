/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef QMuscleSupport_h
#define QMuscleSupport_h

#if QT_VERSION >= 0x040000
# include <qhash.h>
#else
# include "util/String.h"
#endif

#include "support/MuscleSupport.h"  // for uint32, etc

#ifdef MUSCLE_USING_OLD_MICROSOFT_COMPILER
# include "util/Hashtable.h"
#endif

namespace muscle {

#ifndef MUSCLE_USING_OLD_MICROSOFT_COMPILER
template <class T> class HashFunctor;
#endif

/** Enables the use of QStrings as keys in a MUSCLE Hashtable. */
template <>
class HashFunctor<QString>
{
public:
   /** Returns a hash code for the given QString object.
     * @param str The QString to calculate a hash code for.
     */
   uint32 operator () (const QString & str) const 
   {
#if QT_VERSION >= 0x040000
      return qHash(str);
#else
      return CStringHashFunc(str.utf8().data());
#endif
   }
};

};  // end namespace muscle

#endif
