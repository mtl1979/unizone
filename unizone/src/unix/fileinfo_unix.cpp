#include <sys/types.h>
#include <sys/stat.h>
#include <qfile.h>
#include <qstring.h>

#include "fileinfo.h"
#include "mimedb.h"

void
UFileInfo::InitMIMEType()
{
	fMIMEType = queryMimeType(getExtension());
}

void
UFileInfo::InitModificationTime()
{
	if ( isValid() )
	{
		struct stat fst;
		// Encode the Unicode filename to local file system character set
		const char * fname = (const char *) QFile::encodeName(fFullName); 
		int ret = stat(fname, &fst);
		if (ret == 0)
		{
			fModificationTime = fst.st_mtime;
		}
		else
		{
			fModificationTime = time(NULL);
		}
		return;
	}
	else
		fModificationTime = time(NULL);
}
