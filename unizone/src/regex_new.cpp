#include "regex_new.h"
#include "debugimpl.h"
#include "util.h"
#include "wstring.h"

void UncompressCharacterGroups(QString &s)
{
	int pos;
	while ((pos = s.find("-")) != -1)
	{
		QChar first = s[pos-1];
		QChar last = s[pos+1];
		QString tmp;
		for (int x = first.unicode(); x <= last.unicode(); x++)
			tmp += QChar(x);
		s.replace(s.mid(pos-1, 3), tmp);
	}
}

void CompressCharacterGroup(QStringList & s)
{
	int first = -1, last = -1;
	int x = 0;
	QChar prevchar;

#ifdef _DEBUG
	WString l = s.join("|");
#endif

	s.sort();
	s.removeDuplicates();

	if (s.count() == 1)
		return;

	x = 0;
	while (x < s.count())
	{
		while (s.at(x).length() == 1 && s.at(x)[0].unicode() - prevchar.unicode() == 1)
		{
			last = x;
			prevchar = s.at(x)[0];
			if (++x == s.count())
				break;
		}
		if (last - first > 2)
		{
			QString p;
			p = s.at(first)[0];
			p += "-";
			p += s.at(last)[0].lower();
			for (int y = last; y >= first; y--)
				s.remove(s.at(y));
			s.insert(first, p);
			x = first;
			first = last = -1;
			prevchar = QChar::null;
		}

		if (x < s.count() && s.at(x).length() == 1)
		{
			first = x;
			prevchar = s.at(x)[0];
		}
		x++;
	}
#ifdef _DEBUG
	WString l2 = s.join("|");
	PRINT("Group in = %S, out = %S\n", l.getBuffer(), l2.getBuffer());
#endif
}

QString ConvertToRegexInternalNew(const QString & s, bool simple, bool isFirst)
{
	int x = 0;
	QString ret;
	if (s[0] == '(' || s[0] == '[') // Already a regex
		return s;
	while(x < s.length())
	{
		if (s[x] == '\\')			// skip \c
		{
			if (x + 1 < s.length())
			{
				if (s[x + 1] != '@')	// convert \@ to @
					ret += s[x];
				x++;
				ret += s[x];
			}
		}
		else if (s[x] == '*')
		{
			ret += simple ? "*" : ".*";
		}
		else if (s[x] == '?')
		{
			ret += simple ? "?" : ".";
		}
		else if (s[x] == ',')
		{
			ret += "|";
		}
		else
		{
			if (IsRegexToken2(s[x], false)) ret += '\\';
			ret += s[x];
		}

		x++;
		// reset
		if (isFirst)
			isFirst = false;
	}
	return ret;
}

bool FindDuplicatesNew(QStringList & l, bool simple)
{
	int pos = 0;
	int pos2 = 0;

	// Convert all to lowercase
	while (pos < l.count())
	{
		QString tmp = l.at(pos);
		if (tmp != tmp.lower())
		{
			l.remove(tmp);
			l.insert(pos, tmp.lower());
		}
		pos++;
	}

	if (l.count() == 1)
		return false;

	// Check start of strings
	pos = 0;
	while (pos < l.at(0).length())
	{
		for (int i = 1; i < l.size(); i++)
		{
			if (pos == min(l.at(i).length(), l.at(0).length()) || l.at(i)[pos] != l.at(0)[pos])
			{
#ifdef _DEBUG
				WString tmp1 = l.at(0);
				WString tmp2 = l.at(i);
				PRINT("item 0 = %S, item i = %S\n", tmp1.getBuffer(), tmp2.getBuffer());
#endif
				goto step2;
			}
		}
		pos++;
	}
step2:
	while (pos2 < l.at(0).length()) // Check end of strings
	{
		for (int i = 1; i < l.size(); i++)
		{
			if (pos2 == min(l.at(i).length(), l.at(0).length()) || l.at(i)[l.at(i).length() - pos2 - 1] != l.at(0)[l.at(0).length() - pos2 - 1])
			{
#ifdef _DEBUG
				WString tmp1 = l.at(0);
				WString tmp2 = l.at(i);
				PRINT("item 0 = %S, item i = %S\n", tmp1.getBuffer(), tmp2.getBuffer());
#endif
				goto step3;
			}
		}
		pos2++;
	}
step3:
	if (pos == 0 && pos2 == 0 && l.count() > 1)
	{
		QStringList templist;

#ifdef _DEBUG
		WString tmp = l.join("|");
		PRINT("l = %S\n", tmp.getBuffer());
#endif
		QString first = l.first();
		QString last = l.last();

		//
		templist = l;
		templist.remove(first);
		if (FindDuplicatesNew(templist, simple))
		{
			templist.prepend(first);
#ifdef _DEBUG
			WString tmp = templist.join("|");
			PRINT("templist = %S\n", tmp.getBuffer());
#endif
			if (FindDuplicatesNew(templist, simple))
			{
				l = templist;
				return true;
			}
			l = templist;
			return true;
		}

		//
		templist.prepend(first);
		templist.remove(last);
		if (FindDuplicatesNew(templist, simple))
		{
			templist.append(last);
#ifdef _DEBUG
			WString tmp = templist.join("|");
			PRINT("templist = %S\n", tmp.getBuffer());
#endif
			if (FindDuplicatesNew(templist, simple))
			{
				l = templist;
				return true;
			}
			l = templist;
			return true;
		}
		return false;
	}
	if (pos > 0 || pos2 > 0)
	{
		QStringList retlist;
		QString ret;
		bool empty = false;
		if (pos > 0)
		{
			ret += ConvertToRegexInternalNew(l.at(0).left(pos), simple, true);
		}
		for (int i = 0; i < l.size(); i++)
		{
			QString temp;
			temp = l.at(i).mid(pos, l.at(i).length() - pos - pos2);
			if (temp.length() > 0)
				retlist.append(ConvertToRegexInternalNew(temp, simple, false));
			else
			{
				empty = true;
			}
		}
		retlist.removeDuplicates();
		
		// First pass
		if (retlist.count() > 1)
		{
			retlist.sort();
			(void) FindDuplicatesNew(retlist, simple);
		}
pass2:
		// Second pass
		if (retlist.count() > 1)
		{
			bool singleChar = true;
			for (int y = 0; y < retlist.count(); y++)
			{
				if (retlist.at(y).find(QRegExp("^\\[[\\x0020-\\x007F]-[\\x0020-\\x007F]\\]$")) == 0)
				{
					QChar first = retlist.at(y)[1];
					QChar last = retlist.at(y)[3];
#ifdef _DEBUG
					WString rin = retlist.join("|");
#endif
					retlist.remove(retlist.at(y));
					for (int z = last.unicode(); z >= first.unicode(); z--)
					{
						retlist.insert(y, QChar(z));
					}
#ifdef _DEBUG
					WString rout = retlist.join("|");
					PRINT("List in = %S, out = %S\n", rin.getBuffer(), rout.getBuffer());
#endif
				}
				if (retlist.at(y).length() > 1)
				{
					singleChar = false;
					break;
				}
				QChar qc = retlist.at(y)[0];
				if (!muscleInRange(qc, (QChar) '0', (QChar) '9') &&
					!muscleInRange(qc, (QChar) 'a', (QChar) 'z') &&
					!muscleInRange(qc, (QChar) 'A', (QChar) 'Z'))
				{
					singleChar = false;
					break;
				}
			}
			if (singleChar)
			{
				ret += "[";
#ifdef _DEBUG
				WString rin = retlist.join("|");
#endif
				CompressCharacterGroup(retlist);
#ifdef _DEBUG
				WString rout = retlist.join("|");
				PRINT("List in = %S, out = %S\n", rin.getBuffer(), rout.getBuffer());
#endif
				for (int y = 0; y < retlist.count(); y++)
				{
					ret += retlist.at(y);
				}
				ret += "]";
			}
			else
			{
				bool merged = false;
#ifdef _DEBUG
				WString tmp = retlist.join("|");
				PRINT("retlist = %S\n", tmp.getBuffer());
#endif
				for (int y = 0; y < retlist.count(); y++)
				{
					if (retlist.at(y).startsWith("(") && retlist.at(y).endsWith(")"))
					{
						QString item = retlist.at(y);
						retlist.remove(item);
						if (item.contains("|") && !item.mid(1).contains("("))
						{
							item = item.mid(1, item.length()-2);
							QStringList joinlist = item.split("|");
							for (int z = 0; z < joinlist.count(); z++)
								retlist.insert(y+z, joinlist.at(z));
						}
						else
							retlist.insert(y, item);
						merged = true;
					}
				}
				if (!merged)
				{
					for (int y = 0; y < retlist.count(); y++)
					{
						if (retlist.at(y).startsWith("[") && retlist.at(y).endsWith("]"))
						{
							QString item = retlist.at(y);
							retlist.remove(item);
							item = item.mid(1, item.length() - 2);
							UncompressCharacterGroups(item);
							for (int z = 0; z < item.length(); z++)
							{
								retlist.insert(y++, item.at(z));
							}
							merged = true;
						}
					}
					if (merged)
					{
#ifdef _DEBUG
						tmp = retlist.join("|");
						PRINT("retlist = %S\n", tmp.getBuffer());
#endif
						goto pass2;
					}
				}
#ifdef _DEBUG
				tmp = retlist.join("|");
				PRINT("retlist = %S\n", tmp.getBuffer());
#endif

				if (merged)
				{
					if (!FindDuplicatesNew(retlist, simple))
					{
						retlist.sort();
						(void) FindDuplicatesNew(retlist, simple);
					}
				}
#ifdef _DEBUG
				tmp = retlist.join("|");
				PRINT("retlist = %S\n", tmp.getBuffer());
#endif
				ret += "(";
				ret += retlist.join("|");
				ret += ")";
			}
			if (empty)
				ret += "?";
		}
		else
		{
			ret += retlist.at(0);
			if (empty) ret += "?";
		}
		if (pos2 > 0)
		{
			ret += ConvertToRegexInternalNew(l.at(0).right(pos2), simple, false);
		}
		l.clear();
		l.append(ret);
		return true;
	}
	return false;
}

void ConvertToRegexNew(QString & s, bool simple)
{
	QString ret;

	if (s.left(2) == "`^") // already a regex
	return;
start:
	if (!simple)
	{
		ret = "`^";
		if (s.count("|") > 0)
		{
			bool found = false;
			QStringList l = s.split("|");
			l.removeDuplicates();
			if (l.count() == 1) // Only one item left
			{
				s = l.at(0);
				goto start; // We need to restart with the only entry as the "original" string
			}
			found = FindDuplicatesNew(l, simple);
			if (found)
			{
				ret += l.at(0);
				ret += "$";
				s = ret;
				return;
			}
			// Fall through
		}
	}

	ret += ConvertToRegexInternalNew(s, simple, true);
	if (!simple)
		ret += "$";
	s = ret;
}
