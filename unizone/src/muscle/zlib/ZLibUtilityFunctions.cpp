/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

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

MessageRef DeflateMessage(const MessageRef & msgRef, int compressionLevel, bool force)
{
   TCHECKPOINT;

   MessageRef ret;
   if ((msgRef())&&(msgRef()->HasName(MUSCLE_ZLIB_FIELD_NAME_STRING) == false))
   {
      MessageRef defMsg = GetMessageFromPool(msgRef()->what);
      ByteBufferRef buf = msgRef()->FlattenToByteBuffer();
      if ((defMsg())&&(buf()))
      {
         Mutex * m = GetGlobalMuscleLock();
         if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
         {
            ZLibCodec * codec = GetZLibCodec(compressionLevel);
            if (codec) buf = codec->Deflate(*buf(), true);
                  else buf.Reset();
            m->Unlock();

            if ((buf())&&(defMsg()->AddFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, FlatCountableRef(buf.GetGeneric(), false)) == B_NO_ERROR)) ret = ((force)||(defMsg()->FlattenedSize() < msgRef()->FlattenedSize())) ? defMsg : msgRef;
         }
      }
   }
   else ret = msgRef;

   return ret;
}

MessageRef InflateMessage(const MessageRef & msgRef)
{
   TCHECKPOINT;

   MessageRef ret;
   FlatCountableRef fcRef;
   if ((msgRef())&&(msgRef()->FindFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, fcRef) == B_NO_ERROR))
   {
      ByteBufferRef buf(fcRef.GetGeneric(), false);
      if (buf())
      {
         MessageRef infMsg = GetMessageFromPool();
         Mutex * m = GetGlobalMuscleLock();
         if ((infMsg())&&(m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
         {
            ZLibCodec * codec = GetZLibCodec(6);  // doesn't matter which compression level we use, all of them can inflate anything
            if (codec) buf = codec->Inflate(*buf());
                  else buf.Reset();
            m->Unlock();

            if ((buf())&&(infMsg()->Unflatten(buf()->GetBuffer(), buf()->GetNumBytes()) == B_NO_ERROR)) 
            {
               infMsg()->what = msgRef()->what;  // do this after Unflatten(), so that the outer 'what' is the one that gets used
               ret = infMsg;
            }
         }
      }
   }
   else ret = msgRef;

   return ret;
}

END_NAMESPACE(muscle);

#endif
