/* This file is Copyright 2003 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

#include "message/Message.h"
#include "system/SetupSystem.h"
#include "zlib/ZLibCodec.h"
#include "zlib/ZLibUtilityFunctions.h"

namespace muscle {

static const String ZLIB_FIELD_NAME = "_zlib";

static ZLibCodec * GetZLibCodec(int level)
{
   static ZLibCodec * codecs[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

   level = muscleClamp(level, 0, 9);
   if (codecs[level] == NULL) 
   {
      codecs[level] = newnothrow ZLibCodec(level);  // demand-allocate
      if (codecs[level] == NULL) WARN_OUT_OF_MEMORY;
   }
   return codecs[level];
}

MessageRef DeflateMessage(MessageRef msgRef, bool force, int compressionLevel)
{
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
   {
      if ((msgRef())&&(msgRef()->HasName(ZLIB_FIELD_NAME) == false))
      {
         ZLibCodec * codec = GetZLibCodec(compressionLevel);
         if (codec)
         {
            uint32 unflattenedSize = msgRef()->FlattenedSize();
            MessageRef defMsg = GetMessageFromPool(msgRef()->what);
            ByteBufferRef buf = GetByteBufferFromPool(unflattenedSize);
            if ((defMsg())&&(buf()))
            {
               msgRef()->Flatten(buf()->GetBuffer());
               buf = codec->Deflate(*buf(), true);
               if ((buf())&&(defMsg()->AddFlat(ZLIB_FIELD_NAME, FlatCountableRef(buf.GetGeneric(), false)) == B_NO_ERROR)) 
               {
                  m->Unlock();
                  return ((force)||(defMsg()->FlattenedSize() < unflattenedSize)) ? defMsg : msgRef;
               }
            }
         }
         msgRef.Reset();  // notify the user that an error occurred
      }
      m->Unlock();
   }
   return msgRef;
}

MessageRef InflateMessage(MessageRef msgRef)
{
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
   {
if (msgRef()) (void) msgRef()->Rename("_zlib6", ZLIB_FIELD_NAME); // XXX temporary hack to retain compatibility with my 6/19 CueStation build... will be removed in a few days!

      FlatCountableRef fcRef;
      if ((msgRef())&&(msgRef()->FindFlat(ZLIB_FIELD_NAME, fcRef) == B_NO_ERROR))
      {
         ByteBufferRef buf(fcRef.GetGeneric(), false);
         if (buf())
         {
            ZLibCodec * codec = GetZLibCodec(6);  // doesn't matter which one we use, any one can inflate anything
            if (codec)
            {
               buf = codec->Inflate(*buf());
               if (buf())
               {
                  MessageRef infMsg = GetMessageFromPool(msgRef()->what);
                  if ((infMsg())&&(infMsg()->Unflatten(buf()->GetBuffer(), buf()->GetNumBytes()) == B_NO_ERROR)) 
                  {
                     m->Unlock();
                     return infMsg;
                  }
               }
            }
         }
         msgRef.Reset();  // notify the user that an error occurred
      }
      m->Unlock();
   }
   return msgRef;
}

};  // end namespace muscle

#endif
