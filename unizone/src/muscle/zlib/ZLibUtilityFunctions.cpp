/* This file is Copyright 2000-2009 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

#include "message/Message.h"
#include "system/SetupSystem.h"
#include "zlib/ZLibCodec.h"
#include "zlib/ZLibUtilityFunctions.h"

namespace muscle {

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

bool IsMessageDeflated(const MessageRef & msgRef) 
{
   return ((msgRef())&&(msgRef()->HasName(MUSCLE_ZLIB_FIELD_NAME_STRING)));
}

ByteBufferRef DeflateByteBuffer(const uint8 * buf, uint32 numBytes, int compressionLevel)
{
   ByteBufferRef ret;
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
   {
      ZLibCodec * codec = GetZLibCodec(compressionLevel);
      if (codec) ret = codec->Deflate(buf, numBytes, true);
      m->Unlock();
   }
   return ret;
}

ByteBufferRef InflateByteBuffer(const uint8 * buf, uint32 numBytes)
{
   ByteBufferRef ret;
   Mutex * m = GetGlobalMuscleLock();
   if ((m)&&(m->Lock() == B_NO_ERROR))  // serialize so that it's thread safe!
   {
      ZLibCodec * codec = GetZLibCodec(6);  // doesn't matter which compression-level/codec we use, any of them can inflate anything
      if (codec) ret = codec->Inflate(buf, numBytes);
      m->Unlock();
   }
   return ret;
}

MessageRef DeflateMessage(const MessageRef & msgRef, int compressionLevel, bool force)
{
   TCHECKPOINT;

   MessageRef ret = msgRef;
   if ((msgRef())&&(msgRef()->HasName(MUSCLE_ZLIB_FIELD_NAME_STRING) == false))
   {
      MessageRef defMsg = GetMessageFromPool(msgRef()->what);
      ByteBufferRef buf = msgRef()->FlattenToByteBuffer();
      if ((defMsg())&&(buf()))
      {
         buf = DeflateByteBuffer(*buf(), compressionLevel);
         if ((buf())&&(defMsg()->AddFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, FlatCountableRef(buf.GetRefCountableRef(), false)) == B_NO_ERROR)&&((force)||(defMsg()->FlattenedSize() < msgRef()->FlattenedSize()))) ret = defMsg;
      }
   }
   return ret;
}

MessageRef InflateMessage(const MessageRef & msgRef)
{
   TCHECKPOINT;

   MessageRef ret;
   ByteBufferRef bufRef;
   if ((msgRef())&&(msgRef()->FindFlat(MUSCLE_ZLIB_FIELD_NAME_STRING, bufRef) == B_NO_ERROR))
   {
      MessageRef infMsg = GetMessageFromPool();
      if (infMsg())
      {
         bufRef = InflateByteBuffer(*bufRef());
         if ((bufRef())&&(infMsg()->UnflattenFromByteBuffer(bufRef) == B_NO_ERROR)) 
         {
            infMsg()->what = msgRef()->what;  // do this after UnflattenFromByteBuffer(), so that the outer 'what' is the one that gets used
            ret = infMsg;
         }
      }
   }
   else ret = msgRef;

   return ret;
}

}; // end namespace muscle

#endif
