#include "util/String.h"
#include "util/Hashtable.h"
#include "util/StringTokenizer.h"

#include <qstring.h>
#include <qfile.h>

#include "mimedb.h"
#include "debugimpl.h"
#include "wfile.h"

using namespace muscle;

#if defined(__sun) && defined(__SVR4)
# include <sys/types.h>
# include <sys/ddi.h>
#endif

Hashtable<String, String> mimeDB;

void ReadDB(const char *file)
{
	QString fn = QFile::decodeName(file);
	if (!WFile::Exists(fn))
		return;
	WFile f;
	PRINT("Opening MIME db %s...\n", file);
	bool b = f.Open(fn, IO_ReadOnly);
	PRINT("Ok.\n");
	if (b)
	{
		char buf[255];
		PRINT("Entering loop...\n");
		while (!f.AtEnd())
		{
			PRINT("Reading entry...\n");
			bzero(buf, 255);
			int nc = f.ReadLine(buf, 255);
			PRINT("Ok, %i characters.\n", nc);

			char *com = strchr(buf, '#');
			if (com)
				*com = '\0';
			char *nl = strchr(buf, 10);
			if (nl)
				*nl = '\0';
			PRINT("Buffer length: %i\n", strlen(buf));
			if (strlen(buf) > 0)
			{
				StringTokenizer tok(buf);
				String type = tok.GetNextToken();
				const char *ext;
				while ((ext = tok.GetNextToken()) != NULL)
				{
					mimeDB.Put(ext, type);
					PRINT("Extension %s has MIME type %s\n", ext, type.Cstr());
				}
			}
		}
	}
	f.Close();
	PRINT("Closed MIME db %s.\n", file);
}

void InitDB()
{
	ReadDB("/etc/mime.types");
	ReadDB("~/.mime.types");
}

QString
queryMimeType(const QString & qe)
{
	static bool dbInit = false;

	if (qe.isEmpty())
		return QString::null;

	String ext((const char *) qe.local8Bit());
	if (!dbInit)
	{
		InitDB();
		dbInit = true;
	}
	PRINT("Querying MIME type %s...\n", ext.Cstr());
	String type("");
	(void) mimeDB.Get(ext, type);
	PRINT("Result: %s\n", type.Cstr());
	return QString::fromLocal8Bit(type.Cstr());
}
