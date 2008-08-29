/* This file is Copyright 2000-2008 Meyer Sound Laboratories Inc.  See the included LICENSE.txt file for details. */

#ifdef WIN32
# include <errno.h>
# include <io.h> 
#else
# include <dirent.h>
# include <sys/stat.h>  // needed for chmod codes under MacOS/X
#endif

#include "system/SystemInfo.h"  // for GetFilePathSeparator()
#include "util/Directory.h"

BEGIN_NAMESPACE(muscle);

#ifdef WIN32

/* Windows implementation of dirent.h Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.

    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.
*/

struct dirent 
{
   char * d_name;
};

typedef struct _DIR
{
   long                handle; /* -1 for failed rewind */
   struct _finddata_t  info;
   struct dirent       result; /* d_name null iff first time */
   char                *name;  /* null-terminated char string */
} DIR;

static DIR * opendir(const char *name)
{
   DIR * dir = NULL;
   if((name)&&(name[0]))
   {
      size_t base_length = strlen(name);
      const char *all = strchr("/\\", name[base_length - 1]) ? "*" : "/*";
      if (((dir = (DIR *)malloc(sizeof *dir)) != NULL) && ((dir->name = (char *) malloc(base_length + strlen(all) + 1)) != NULL))
      {
         strcat(strcpy(dir->name, name), all);
         if((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1) dir->result.d_name = NULL;
         else 
         {
            free(dir->name);
            free(dir);
            dir = 0;
         }
      }
      else 
      {
         free(dir);
         dir   = 0;
         errno = ENOMEM;
      }
   }
   else errno = EINVAL;
   return dir;
}

static int closedir(DIR *dir)
{
   int result = -1;
   if (dir)
   {
      if (dir->handle != -1) result = _findclose(dir->handle);
      free(dir->name);
      free(dir);
   }
   if (result == -1) errno = EBADF;
   return result;
}

static struct dirent * readdir(DIR *dir)
{
   struct dirent * result = NULL;
   if(dir && dir->handle != -1)
   {
      if((!dir->result.d_name)||(_findnext(dir->handle, &dir->info) != -1))
      {
         result         = &dir->result;
         result->d_name = dir->info.name;
      }
   }
   else errno = EBADF;
   return result;
}

static void rewinddir(DIR *dir)
{
   if((dir)&&(dir->handle != -1))
   {
      _findclose(dir->handle);
      dir->handle = (long) _findfirst(dir->name, &dir->info);
      dir->result.d_name = 0;
   }
   else errno = EBADF;
}
#endif

void Directory :: operator++(int)
{
   DIR * dir = (DIR *) _dirPtr;
   struct dirent * entry = dir ? readdir(dir) : NULL;
   _currentFileName = entry ? entry->d_name : NULL;
}

void Directory :: Rewind() 
{
   if (_dirPtr) rewinddir((DIR *)_dirPtr); 
   (*this)++;
}

void Directory :: Reset()
{
   if (_dirPtr) closedir((DIR *)_dirPtr);
   _dirPtr = NULL;
}

status_t Directory :: SetDir(const char * dirPath)
{
   Reset();
   if (dirPath)
   {
      _dirPtr = opendir(dirPath);
      if (_dirPtr == NULL) return B_ERROR;

      (*this)++;   // make the first entry in the directory the current entry.
      return B_NO_ERROR;
   }
   else return B_ERROR;
}

status_t Directory :: CreateDirectory(const char * dirPath, bool forceCreateParentDirsIfNecessary)
{
   if (forceCreateParentDirsIfNecessary)
   {
      char sep = *GetFilePathSeparator();  // technically cheating but I don't want to have to write strrstr()
      const char * lastSlash = strrchr(dirPath+((dirPath[0]==sep)?1:0), sep);
      if (lastSlash)
      {
         uint32 subLen = lastSlash-dirPath;
         char * temp = newnothrow_array(char, subLen+1);
         if (temp == NULL) {WARN_OUT_OF_MEMORY; return B_ERROR;}

         strncpy(temp, dirPath, subLen);
         temp[subLen] = '\0';

         Directory pd(temp);
         if ((pd.IsValid() == false)&&(Directory::CreateDirectory(temp, forceCreateParentDirsIfNecessary) != B_NO_ERROR))
         {
            delete [] temp;
            return B_ERROR;
         }
         else delete [] temp;
      }
   }

   // base case!
#ifdef WIN32
   return CreateDirectoryA(dirPath, NULL) ? B_NO_ERROR : B_ERROR;
#else
   return (mkdir(dirPath, S_IRWXU|S_IRWXG|S_IRWXO) == 0) ? B_NO_ERROR : B_ERROR;
#endif
}
END_NAMESPACE(muscle);
