// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include "ulistview.h"
#include "settings.h"

WUniListItem::WUniListItem(
						   QListView * parent, 
						   QString a, QString b, QString c, QString d,
						   QString e, QString f, QString g, QString h, QString i 
						   ) 
						   : QListViewItem(parent, a, b, c, d, e, f, g, h)
{
	
	
	// Store sort keys and initialize text property
	
	setText(0,a);
	setText(1,b);
	setText(2,c);
	setText(3,d);
	setText(4,e);
	setText(5,f);
	setText(6,g);
	setText(7,h);
	setText(8,i);
	
	// Set default column types to 'Generic'
	
	setColumnType(0, ColumnType::Generic);
	setColumnType(1, ColumnType::Generic);
	setColumnType(2, ColumnType::Generic);
	setColumnType(3, ColumnType::Generic);
	setColumnType(4, ColumnType::Generic);
	setColumnType(5, ColumnType::Generic);
	setColumnType(6, ColumnType::Generic);
	setColumnType(7, ColumnType::Generic);
	setColumnType(8, ColumnType::Generic);
}

// if more constructors are needed, they will be added later

// Updates sort key for other functions

void 
WUniListItem::setText(int col, const QString & text)
{
	fKey[col] = text;					// insensitive sort
	QListViewItem::setText(col, text);	// pass it on
}

// Returns sort key by converting to hexadecimal numeric value or ten spaces

QString 
WUniListItem::key(int c, bool asc) const
{
	int n, m;
	QString result, q1, q2;
	switch (UColumnType[c])
	{
	case Number:
	case Percentage:
	case Date:
	case Size:
	case Time:
	case TransferSpeed:
		result = fKey[c];
		PRINT("\tRESULT STARTS AS\t %s\n", result.latin1());
		bool ok;
		n = result.toLong(&ok);
		if (ok)
		{
			// convert our number to hexadecimal! what a thought, huh?
			result.sprintf("0x%08x", n);
			PRINT("\tRESULT IS %s\n", result.latin1());
		}
		return result;
	case TransferLoad:
		q1 = fKey[c].left(fKey[c].find(","));
		q2 = fKey[c].mid(fKey[c].find(",")+1);
		bool ok1,ok2;
		n = q1.toInt(&ok1);
		m = q2.toInt(&ok2);
		if (ok1 && ok2)
		{
			int o = 0;
			if (m == WSettings::Unlimited)
				o = 0xFFFF0000 + n; 
			else if (m == 0)
				return "          ";
			else if (n == 0)
				o = m;
			else
			{
				o = (int) (double) ( (double) n / (double) m * 10000.0f );
				o = o * 100 + m;
			}
			result.sprintf("0x%08x", o);
		}
		else
		{
			return "0x00000000";
		}
		return result;
	case String_NoCase:
		return text(c).lower();
	case String_NoCase_Stripped:
		return StripURL(text(c)).lower();
	case String_Cased_Stripped:
		return StripURL(text(c));
	case ConnectionSpeed:
		q1 = QListViewItem::text(c);
		if (q1 == "300 baud") 
			return "0x01";
		else if (q1 == "14.4 kbps") 
			return "0x02";
		else if (q1 == "28.8 kbps") 
			return "0x03";
		else if ((q1 == "33.6 kbps") || (q1 == "36.6 kbps")) 
			return "0x04";
		else if (q1 == "57.6 kbps") 
			return "0x05";
		else if (q1 == "ISDN-64k") 
			return "0x06";
		else if (q1 == "ISDN-128k") 
			return "0x07";
		else if (q1 == "DSL") 
			return "0x08";
		else if (q1 == "Cable") 
			return "0x09";
		else if (q1 == "T1") 
			return "0x0A";
		else if (q1 == "T3") 
			return "0x0B";
		else if (q1 == "OC-3") 
			return "0x0C";
		else if (q1 == "OC-12") 
			return "0x0D";
		else 
			return "0x00";
	default:
		return text(c);
	}
}

// Returns numeric representation of requested item, can be used for sorting
// If can't be converted to numeric value, returns -1

long
WUniListItem::item(int c)
{
	long n, m, o;
	QString q1, q2;
	switch (UColumnType[c])
	{
	case Number:
	case Percentage:
	case Date:
	case Time:
	case Size:
		n = fKey[c].toLong();
		return n;
	case TransferSpeed:
		n = (long) fKey[c].toDouble();
		return n;
	case TransferLoad:
		q1 = fKey[c].left(fKey[c].find(","));
		q2 = fKey[c].mid(fKey[c].find(",")+1);
		bool ok1,ok2;
		n = q1.toInt(&ok1);
		m = q2.toInt(&ok2);
		if (ok1 && ok2)
		{
			o = 0;
			if (m == WSettings::Unlimited)
				o = 0xFFFF0000 + n; 
			else if (m == 0)
				return -1;
			else if (n == 0)
				o = m;
			else
			{
				o = (int) (double) ( (double) n / (double) m * 10000.0f );
				o = o * 100 + m;
			}
			return o;
		}
		else
		{
			return 0;
		}
	case ConnectionSpeed:
		q1 = QListViewItem::text(c);
		if (q1 == "300 baud") 
			return 1;
		else if (q1 == "14.4 kbps") 
			return 2;
		else if (q1 == "28.8 kbps") 
			return 3;
		else if ((q1 == "33.6 kbps") || (q1 == "36.6 kbps")) 
			return 4;
		else if (q1 == "57.6 kbps") 
			return 5;
		else if (q1 == "ISDN-64k") 
			return 6;
		else if (q1 == "ISDN-128k") 
			return 7;
		else if (q1 == "DSL") 
			return 8;
		else if (q1 == "Cable") 
			return 9;
		else if (q1 == "T1") 
			return 10;
		else if (q1 == "T3") 
			return 11;
		else if (q1 == "OC-3") 
			return 12;
		else if (q1 == "OC-12") 
			return 13;
		else 
			return 0;
	default:
		return -1;
	}
}

QString 
WUniListItem::text(int c) const
{
	QString postFix, result, q1, q2;
	double n;
	double o;
	int p, q;
	long lMod;
	uint32 secs, min, hours;
	bool ok;
	switch (UColumnType[c])
	{
	case String_NoCase_Stripped:
	case String_Cased_Stripped:
		return StripURL(QListViewItem::text(c));
	case Percentage:
		return QListViewItem::text(c)+" %";
	case Size:
		result = QListViewItem::text(c);
		n = (double)result.toLong(&ok);
		postFix = "B";
		if (ok)
		{
			if (n >= 1024.0f)	// > 1 kB?
			{
				n /= 1024.0f;
				postFix = "kB";	// we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB
				
				if (n >= 1024.0f)	// > 1 MB?
				{
					n /= 1024.0f;
					postFix = "MB";
					
					if (n >= 1024.0f)	// > 1 GB?
					{
						n /= 1024.0f;
						postFix = "GB";
					}
				}
			}
		}
		else
			n = 0.0f;
		result.sprintf("%.2f %s", n, postFix.latin1());
		return result;
		
	case TransferSpeed:
		result = QListViewItem::text(c);
		n = result.toDouble(&ok);
		postFix = "B/s";
		if (ok)
		{
			if (n >= 1024.0f)	// > 1 kB?
			{
				n /= 1024.0f;
				postFix = "kB/s";	// we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB
				
				if (n >= 1024.0f)	// > 1 MB?
				{
					n /= 1024.0f;
					postFix = "MB/s";
					
					if (n >= 1024.0f)	// > 1 GB?
					{
						n /= 1024.0f;
						postFix = "GB/s";
					}
				}
			}
		}
		else
			n = 0.0f;
		result.sprintf("%.2f %s", n, postFix.latin1());
		return result;
		
		
	case TransferLoad:
		q1 = fKey[c].left(fKey[c].find(","));
		q2 = fKey[c].mid(fKey[c].find(",")+1);
		bool ok1,ok2;
		p = q1.toInt(&ok1);
		q = q2.toInt(&ok2);
		if (ok1 && ok2)
		{
			o = 0.0f;
			if (q == WSettings::Unlimited)
			{
				result.sprintf("(%d/-)",p);
				return result;
			}
			if (q == 0)
				return QString::null;
			if (p == 0)
				o = 0.0f;
			else
				o = (double) ( (double) p / (double) q * 100.0f );
			result.sprintf("(%d/%d) %3.2f %%", p, q, o);
		}
		return result;
		
	case Date:
		lMod = fKey[c].toLong();
		result = ctime((const time_t *)&lMod);
#ifdef WIN32
		result = result.left(result.length()-1);
#endif
		return result;
	case Time:
		lMod = fKey[c].toLong();
		secs = lMod;
		min = secs / 60; secs = secs % 60;
		hours = min / 60; min = min % 60;
		
		result.sprintf("%d:%.2d:%.2d", hours, min, secs);
		return result;
		
	default:
		return QListViewItem::text(c);
	}
}

void 
WUniListItem::setColumnType(int c, WUniListItem::ColumnType ct)
{
	UColumnType[c] = ct;
}

WUniListItem::ColumnType
WUniListItem::columnType(int c)
{
	return UColumnType[c];
}

// set/get user colors
void 
WUniListItem::setRowBaseColor(int i, QColor & color) 
{
	RowBaseColor[i] = color;
}

QColor 
WUniListItem::rowBaseColor(int i)
{
	return RowBaseColor[i];
}

void
WUniListItem::setRowTextColor(int i, QColor & color)
{
	RowTextColor[i] = color;
}

QColor
WUniListItem::rowTextColor(int i)
{
	return RowTextColor[i];
}
