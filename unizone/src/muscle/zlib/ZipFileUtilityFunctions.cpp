/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifdef MUSCLE_ENABLE_ZLIB_ENCODING

#include "util/ByteBuffer.h"
#include "util/MiscUtilityFunctions.h"
#include "util/StringTokenizer.h"
#include "zlib/ZipFileUtilityFunctions.h"
#include "zlib/zlib/contrib/minizip/zip.h"
#include "zlib/zlib/contrib/minizip/unzip.h"

BEGIN_NAMESPACE(muscle);

static status_t WriteZipFileAux(zipFile zf, const String & baseName, const Message & msg, int compressionLevel, zip_fileinfo * fileInfo)
{
   MessageFieldNameIterator iter = msg.GetFieldNameIterator();
   const String * fn;
   while((fn = iter.GetNextFieldNameString()) != NULL)
   {
      uint32 fieldType;
      if (msg.GetInfo(*fn, &fieldType) != B_NO_ERROR) return B_ERROR;
      switch(fieldType)
      {
         case B_MESSAGE_TYPE:
         {
            String newBaseName = baseName;
            if ((newBaseName.HasChars())&&(newBaseName.EndsWith('/') == false)) newBaseName += '/';
            newBaseName += *fn;

            // Message fields we treat as sub-directories   
            MessageRef subMsg;
            for (int32 i=0; msg.FindMessage(*fn, i, subMsg) == B_NO_ERROR; i++) if (WriteZipFileAux(zf, newBaseName, *subMsg(), compressionLevel, fileInfo) != B_NO_ERROR) return B_ERROR;
         }
         break;

         case B_RAW_TYPE:
         {
            String fileName = baseName;
            if ((fileName.HasChars())&&(fileName.EndsWith('/') == false)) fileName += '/';
            fileName += *fn;

            const void * data;
            uint32 numBytes;
            for (int32 i=0; msg.FindData(*fn, B_RAW_TYPE, i, &data, &numBytes) == B_NO_ERROR; i++)
            {
               if (zipOpenNewFileInZip2(zf,
                                        fileName(),  // file name
                                        fileInfo,    // file info
                                        NULL,        // const void* extrafield_local,
                                        0,           // uInt size_extrafield_local,
                                        NULL,        // const void* extrafield_global,
                                        0,           // uInt size_extrafield_global,
                                        NULL,        // const char* comment,
                                        (compressionLevel>0)?Z_DEFLATED:0,  // int method,
                                        compressionLevel,  // int compressionLevel
                                        0) != ZIP_OK) return B_ERROR;
               if (zipWriteInFileInZip(zf, data, numBytes) != ZIP_OK) return B_ERROR;
               if (zipCloseFileInZip(zf) != ZIP_OK) return B_ERROR;
            }
         }
         break;
      }
   }
   return B_NO_ERROR;
}

status_t WriteZipFile(const char * fileName, const Message & msg, int compressionLevel, uint64 fileCreationTime)
{
   TCHECKPOINT;

   zipFile zf = zipOpen(fileName, 0);
   if (zf)
   {
      zip_fileinfo * fi = NULL;
      zip_fileinfo fileInfo;  
      {
         memset(&fileInfo, 0, sizeof(fileInfo));
         int year, month, day, hour, minute, second;
         if (GetHumanReadableTimeValues((fileCreationTime==MUSCLE_TIME_NEVER)?GetCurrentTime64(MUSCLE_TIMEZONE_LOCAL):fileCreationTime, year, month, day, hour, minute, second, MUSCLE_TIMEZONE_LOCAL) == B_NO_ERROR)
         {
            fi = &fileInfo;
            fileInfo.tmz_date.tm_sec  = second;
            fileInfo.tmz_date.tm_min  = minute;
            fileInfo.tmz_date.tm_hour = hour;
            fileInfo.tmz_date.tm_mday = day+1;
            fileInfo.tmz_date.tm_mon  = month;
            fileInfo.tmz_date.tm_year = year;
         }
      }
      
      status_t ret = WriteZipFileAux(zf, "", msg, compressionLevel, fi);
      zipClose(zf, NULL);
      return ret; 
   }
   else return B_ERROR;
}

static status_t ReadZipFileAux(zipFile zf, Message & msg, char * nameBuf, uint32 nameBufLen)
{
   while(unzOpenCurrentFile(zf) == UNZ_OK)
   {
      unz_file_info fileInfo;
      if (unzGetCurrentFileInfo(zf, &fileInfo, nameBuf, nameBufLen, NULL, 0, NULL, 0) != UNZ_OK) return B_ERROR;

      // Add the new entry to the appropriate spot in the tree (demand-allocate sub-Messages as necessary)
      {
         const char * nulByte = strchr(nameBuf, '\0');
         bool isFolder = ((nulByte > nameBuf)&&(*(nulByte-1) == '/'));
         Message * m = &msg;
         StringTokenizer tok(true, nameBuf, "/");
         const char * nextTok;
         while((nextTok = tok()) != NULL)
         {
            String fn(nextTok);
            if ((isFolder)||(tok.GetRemainderOfString()))
            {
               // Demand-allocate a sub-message
               MessageRef subMsg;
               if (m->FindMessage(fn, subMsg) != B_NO_ERROR) 
               {
                  if ((m->AddMessage(fn, Message()) != B_NO_ERROR)||(m->FindMessage(fn, subMsg) != B_NO_ERROR)) return B_ERROR;
               }
               m = subMsg();
            }
            else
            {
               ByteBufferRef bufRef = GetByteBufferFromPool(fileInfo.uncompressed_size);
               if ((bufRef() == NULL)||(unzReadCurrentFile(zf, bufRef()->GetBuffer(), bufRef()->GetNumBytes()) != (int32)bufRef()->GetNumBytes())) return B_ERROR;

               if (m->AddFlat(fn, FlatCountableRef(bufRef.GetGeneric(), true)) != B_NO_ERROR) return B_ERROR;
            }
         }
      }
      if (unzCloseCurrentFile(zf) != UNZ_OK) return B_ERROR;
      if (unzGoToNextFile(zf) != UNZ_OK) break;
   }
   return B_NO_ERROR;
}

MessageRef ReadZipFile(const char * fileName)
{
   TCHECKPOINT;

   static const int NAME_BUF_LEN = 8*1024;  // names longer than 8KB are ridiculous anyway!
   char * nameBuf = newnothrow_array(char, NAME_BUF_LEN);
   if (nameBuf)
   {
      MessageRef ret = GetMessageFromPool();
      if (ret())
      {
         zipFile zf = unzOpen(fileName);
         if (zf != NULL)
         {
            if (ReadZipFileAux(zf, *ret(), nameBuf, NAME_BUF_LEN) != B_NO_ERROR) ret.Reset();
            unzClose(zf);
         }
         else ret.Reset();  // failure!
      }
      delete [] nameBuf;
      return ret;
   }
   else WARN_OUT_OF_MEMORY;

   return MessageRef();
}

END_NAMESPACE(muscle);

#endif
