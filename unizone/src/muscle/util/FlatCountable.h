/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifndef	MuscleFlatCountable_h
#define	MuscleFlatCountable_h

#include "support/Flattenable.h"
#include "util/RefCount.h"

BEGIN_NAMESPACE(muscle);

/** This class is used simply to more succinctly designate any
 *  object that derives from both Flattenable and RefCountable.
 *  That is, it is an interface for objects that can be both
 *  flattened and reference-counted.
 */
class FlatCountable : public RefCountable, public Flattenable
{
public:
   /** Default ctor */
   FlatCountable() {/* empty */}

   /** Dtor */
   virtual ~FlatCountable() {/* empty */}
};
DECLARE_REFTYPES(FlatCountable);

END_NAMESPACE(muscle);

#endif 
