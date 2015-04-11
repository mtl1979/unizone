#include "regex_old.h"
#include "debugimpl.h"
#include "util.h"
#include "wstring.h"

QString ConvertToRegexInternalOld(const QString & s, bool simple, bool isFirst)
{
	int x = 0;
	QString ret;
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

bool FindDuplicatesOld(QStringList & l, bool simple)
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

	pos = 0;
	while (pos < l.at(0).length()) // Check start of strings
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
	if (pos > 0 || pos2 > 0)
	{
		QStringList retlist;
		QString ret;
restart1:
		if (pos > 0)
		{
			ret += ConvertToRegexInternalOld(l.at(0).left(pos), simple, true);
		}
		for (int i = 0; i < l.size(); i++)
		{
			QString temp;
restart2:
			temp = l.at(i).mid(pos, l.at(i).length() - pos - pos2);
			if (temp.length() > 0)
				retlist.append(ConvertToRegexInternalOld(temp, simple, false));
			else
			{
				// We try to expand the middle section by one character and then restart
				retlist.clear();
				if (pos2 > 0)
				{
					i = 0;
					pos2--;
					goto restart2;
				}
				else if (pos > 0)
				{
					pos--;
					ret = "^"; // Re-initialize
					goto restart1;
				}
			}
		}
		retlist.removeDuplicates();
		if (retlist.count() > 1)
		{
			bool singleChar = true;
			for (int y = 0; y < retlist.count(); y++)
			{
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
				retlist.sort();
				for (int y = 0; y < retlist.count(); y++)
				{
					QChar qc = retlist.at(y)[0];
					if (muscleInRange(qc, (QChar) '0', (QChar) '9'))
						ret += qc;
					else
					{
						ret += qc.lower();
					}
				}
				ret += "]";
			}
			else
			{
				ret += "(";
				ret += retlist.join("|");
				ret += ")";
			}
		}
		else
		{
			ret += retlist.at(0);
		}
        if (pos2 > 0)
		{
			ret += ConvertToRegexInternalOld(l.at(0).right(pos2), simple, false);
		}
		l.clear();
		l.append(ret);
		return true;
	}
	return false;
}

void ConvertToRegexOld(QString & s, bool simple)
{
	QString ret;
start:
	if (!simple)
	{
		ret = "^";
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
			found = FindDuplicatesOld(l, simple);
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

	ret += ConvertToRegexInternalOld(s, simple, true);
	if (!simple)
		ret += "$";
	s = ret;
}
