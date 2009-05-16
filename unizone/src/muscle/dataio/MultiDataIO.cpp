#include "dataio/MultiDataIO.h"

namespace muscle {
 
int32 MultiDataIO :: Read(void * buffer, uint32 size)
{
   if (HasChildren())
   {
      int32 ret = GetFirstChild()->Read(buffer, size);
           if (ret < 0) return -1;
      else if (ret > 0) return (SeekAll(1, ret, IO_SEEK_CUR)==B_NO_ERROR) ? ret : -1;
   }
   return 0;
}

int32 MultiDataIO :: Write(const void * buffer, uint32 size) 
{
   int32 ret = 0;
   int32 firstWriteBytes = -1;
   for (uint32 i=0; i<_childIOs.GetNumItems(); i++)
   {
      int32 subRet = _childIOs[i]()->Write(buffer, size);
      if (subRet < 0) return -1;
      if (firstWriteBytes < 0) firstWriteBytes = size = ret = subRet;
      if (subRet != firstWriteBytes) return -1;  // we can't handle the children writing differing amounts!
   }
   return ret;
}

void MultiDataIO :: FlushOutput() 
{
   for (uint32 i=0; i<_childIOs.GetNumItems(); i++) _childIOs[i]()->FlushOutput();
}

void MultiDataIO :: WriteBufferedOutput() 
{
   for (uint32 i=0; i<_childIOs.GetNumItems(); i++) _childIOs[i]()->WriteBufferedOutput();
}

bool MultiDataIO :: HasBufferedOutput() const 
{
   for (uint32 i=0; i<_childIOs.GetNumItems(); i++) if (_childIOs[i]()->HasBufferedOutput()) return true;
   return false;
}

status_t MultiDataIO :: SeekAll(uint32 first, int64 offset, int whence)
{
   for (uint32 i=first; i<_childIOs.GetNumItems(); i++) if (_childIOs[i]()->Seek(offset, whence) != B_NO_ERROR) return B_ERROR;
   return B_NO_ERROR;
}

}; // end namespace muscle
