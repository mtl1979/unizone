/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

#include "message/Message.h"
#include "system/SetupSystem.h"
#include "zlib/ZLibCodec.h"
#include "zlib/ZLibUtilityFunctions.h"

BEGIN_NAMESPACE(muscle);

static const String MUSCLE_ZLIB_FIELD_NAME_STRING = MUSCLE_ZLIB_FIELD_NAME;

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

MessageRef DeflateMessage(MessageRef msgRef, int compressionLevel, bool force)
{
   if ((msgRef())&&(msgRef()->HasName(MUSCLE_ZLIB_FIELD_NAME_STRING) == false))
   {
      uint32 inflatedSize = msgRef()->FlattenedSize();
      MessageRef defMsg = GetMessageFromPool(msgRef()->what);
      ByteBufferRef buf = GetByteBufferFromPool(inflatedSize);

      if ((defMsg())&&(buf()))
      {
         msgRef()->Flatten(buf()->GetBuffer());

         Mutex * m = GetGlobalMuscleLock();
         if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
         {
            ZLibCodec * codec = GetZLibCodec(compressionLevel);
            if (codec)
            {
               buf = codec->Deflate(*buf(), true);
               if ((buf())&&(defMsg()->AddFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, FlatCountableRef(buf.GetGeneric(), false)) == B_NO_ERROR)) 
               {
                  m->Unlock();
                  return ((force)||(defMsg()->FlattenedSize() < inflatedSize)) ? defMsg : msgRef;
               }
            }
            m->Unlock();
         }
      }
      msgRef.Reset();  // notify the caller that an error occurred
   }
   return msgRef;
}

MessageRef InflateMessage(MessageRef msgRef)
{
   FlatCountableRef fcRef;
   if ((msgRef())&&(msgRef()->FindFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, fcRef) == B_NO_ERROR))
   {
      ByteBufferRef buf(fcRef.GetGeneric(), false);
      if (buf())
      {
         Mutex * m = GetGlobalMuscleLock();
         if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
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
            m->Unlock();
         }
      }
      msgRef.Reset();  // notify the caller that an error occurred
   }
   return msgRef;
}

END_NAMESPACE(muscle);

#endif
