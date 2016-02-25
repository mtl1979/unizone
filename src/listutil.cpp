#include "listutil.h"
#include "tokenizer.h"
#include "util/StringTokenizer.h"

void
AddToList(String & slist, const String &item)
{
	if (slist.Length() == 0)
		slist = item.Trim();
	else
	{
		slist += ",";
		slist += item.Trim();
	}
}

void
AddToList(String & slist, const char *item)
{
	AddToList(slist, String(item));
}

void AddToList(QString &slist, const QString &entry)
{
	if (slist.isEmpty())
		slist = entry.stripWhiteSpace();
	else
	{
		slist += ",";
		slist += entry.stripWhiteSpace();
	}
}

void RemoveFromList(QString &slist, const QString &entry)
{
	if (slist == entry.stripWhiteSpace())
	{
		slist = QString::null;
		return;
	}

	QStringList list = QStringList::split(",", slist);
	QStringList::Iterator iter = list.begin();
	while (iter != list.end())
	{
		if ((*iter).lower() == entry.stripWhiteSpace().lower())
		{
			list.remove(iter);
			break;
		}
		iter++;
	}
	slist = list.join(",");
}

void RemoveFromList(String &slist, const String &entry)
{
	if (slist == entry.Trim())
	{
		slist = "";
		return;
	}

	StringTokenizer tok(slist.Cstr(), ",");
	String out;
	const char * tmp;
	while ((tmp = tok.GetNextToken()) != NULL)
	{
		if (!entry.Trim().EqualsIgnoreCase(tmp))
			AddToList(out, tmp);
	}
	slist = out;
}

bool
Contains(const QString &slist, const QString &entry)
{
	QStringTokenizer tok(slist,",");
	QString t;
	while ((t = tok.GetNextToken()) != QString::null)
	{
		if (t == entry)
			return true;
	}
	return false;
}
