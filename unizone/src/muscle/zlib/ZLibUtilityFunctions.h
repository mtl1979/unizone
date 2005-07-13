/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef ZLibUtilityFunctions_h
#define ZLibUtilityFunctions_h

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

#include "message/Message.h"

BEGIN_NAMESPACE(muscle);

/** Examines the contents of the given Message, and creates and returns a new
 *  Message that represents the same data as the given Message, but in compressed form.
 *  If the passed-in Message is already in compressed form (i.e. it was created by
 *  a previous call to DeflateMessage()), or if the deflation didn't decrease the size
 *  any, then a reference to the original passed-in Message is returned instead.
 *  The returned Message is guaranteed to have the same 'what' code as the passed-in Message.
 *  If there is an error (out of memory?), a NULL reference is returned.
 *  @param msgRef The Message to create a compact version of.
 *  @param msgRef Reference to the newly generated compressed Message, or to the passed
 *                in Message, or a NULL reference on failure.
 *  @param compressionLevel The level of ZLib compression to use when creating the
 *                          compressed Message.  Should be between 0 (no compression)
 *                          and 9 (maximum compression).  Default value is 6.
 *  @param force If true, we will return a compressed Message even if the compressed Message's
 *               size is bigger than that of the original(!).  Otherwise, we'll return the
 *               original Message if the compression didn't actually make the Message's flattened
 *               size smaller.  Defaults to true.
 */
MessageRef DeflateMessage(const MessageRef & msgRef, int compressionLevel = 6, bool force=true);

/** Examines the given Message, and if it is a Message in compressed form (i.e. one
 *  that was previously created by DeflateMessage()), creates and returns the
 *  equivalent uncompressed Message.  If the passed-in Message was not in compressed
 *  form, then this function just returns a reference to the original passed-in Message.
 *  The returned Message is guaranteed to have the same 'what' code as the passed-in Message.
 *  Returns a NULL reference on failure (out of memory?)
 *  @param msgRef Message to examine and make an uncompressed equivalent of.
 *  @return Reference to an uncompressed Message on success, or a NULL reference on failure.
 */
MessageRef InflateMessage(const MessageRef & msgRef);

// This is the field name that we store deflated data into
#define MUSCLE_ZLIB_FIELD_NAME "_zlib"

END_NAMESPACE(muscle);

#endif

#endif
