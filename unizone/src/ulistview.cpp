// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include <qapplication.h>
#include <q3dragobject.h>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QImageReader>

#include "ulistview.h"
#include "settings.h"
#include "debugimpl.h"
#include "util.h"
#include "wstring.h"
#include "global.h"

WUniListItem::WUniListItem(
						   Q3ListView * parent,
						   QString a, QString b, QString c, QString d, QString e,
						   QString f, QString g, QString h, QString i, QString j
						   )
						   : Q3ListViewItem(parent)
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
	setText(9,j);

	// Set default column types to 'Generic'

	setColumnType(0, Generic);
	setColumnType(1, Generic);
	setColumnType(2, Generic);
	setColumnType(3, Generic);
	setColumnType(4, Generic);
	setColumnType(5, Generic);
	setColumnType(6, Generic);
	setColumnType(7, Generic);
	setColumnType(8, Generic);
	setColumnType(9, Generic);
}

// if more constructors are needed, they will be added later

// Updates sort key for other functions

void
WUniListItem::setText(int col, const QString & text)
{
	fKey[col] = text;					// insensitive sort
	Q3ListViewItem::setText(col, text);	// pass it on
}

// Returns sort key by converting to hexadecimal numeric value or ten spaces

QString
WUniListItem::key(int c, bool /* asc */) const
{
	int64 n, m;
	int32 bw;
	QString result, q1, q2;

#ifdef _DEBUG
	WString wres;
#endif

	switch (UColumnType[c])
	{
	case Number:
	case Percentage:
	case Date:
	case Size:
	case Time:
		{
			result = fKey[c];
#ifdef _DEBUG
			wres = result;
			PRINT("\tRESULT STARTS AS\t %S\n", wres.getBuffer());
#endif

			bool ok;
			n = toLongLong(result, &ok);
			if (ok)
			{
				// convert our number to hexadecimal! what a thought, huh?
				result = hexFromLongLong(n, 16);
#ifdef _DEBUG
				wres = result;
				PRINT("\tRESULT IS %S\n", wres.getBuffer());
#endif
			}
			return result;
		}
	case TransferSpeed:
		{
			result = fKey[c];
#ifdef _DEBUG
			wres = result;
			PRINT("\tRESULT STARTS AS\t %S\n", wres.getBuffer());
#endif

			bool ok;
			n = llrint(result.toDouble(&ok)); // We need to convert from double to long long
			if (ok)
			{
				// convert our number to hexadecimal! what a thought, huh?
				result = hexFromLongLong(n, 16);
#ifdef _DEBUG
				wres = result;
				PRINT("\tRESULT IS %S\n", wres.getBuffer());
#endif
			}
			return result;
		}
	case TransferLoad:
		{
			q1 = fKey[c].left(fKey[c].find(","));
			q2 = fKey[c].mid(fKey[c].find(",")+1);
			bool ok1,ok2;
			n = toLongLong(q1, &ok1);
			m = toLongLong(q2, &ok2);
			if (ok1 && ok2)
			{
				int64 o = 0;
				if (m == WSettings::Unlimited)
					o = 0xFFFF0000 + n;
				else if (m == 0)
					return "          ";
				else if (n == 0)
					o = m;
				else
				{
					o = lrint((double) ( (double) n / (double) m * 10000.0f ));
					o = o * 100 + m;
				}
				result.sprintf("0x%08llx", o);
			}
			else
			{
				return "0x00000000";
			}
			return result;
		}
	case String_NoCase:
		{
			return text(c).lower();
		}
	case String_NoCase_Stripped:
		{
			return StripURL(text(c)).lower();
		}
	case String_Cased_Stripped:
		{
			return StripURL(text(c));
		}
	case ConnectionSpeed:
		{
			bw = BandwidthToBytes(Q3ListViewItem::text(c));
			switch (bw)
			{
			case 75:
			case 300:
				return "0x01";
			case 14400:
				return "0x02";
			case 28800:
				return "0x03";
			case 33600:
				return "0x04";
			case 57600:
				return "0x05";
			case 64000:
				return "0x06";
			case 128000:
				return "0x07";
			case 256000:
				return "0x08";
			case 384000:
				return "0x09";
			case 512000:
				return "0x0A";
			case 768000:
				return "0x0B";
			case 1000000:
			case 1024000:
				return "0x0C";
			case 1500000:
				return "0x0D";
			case 4500000:
				return "0x0E";
			case 155520000:
				return "0x0F";
			case 622080000:
				return "0x10";
			default:
				return "0x00";
			}
		}
	default:
		{
			return text(c);
		}
	}
}

// Returns numeric representation of requested item, can be used for sorting
// If can't be converted to numeric value, returns -1

int64
WUniListItem::item(int c)
{
	int64 n, m, o;
	int32 bw;
	QString q1, q2;
	switch (UColumnType[c])
	{
	case Number:
	case Percentage:
	case Date:
	case Time:
	case Size:
		{
		n = toLongLong(fKey[c]);
		return n;
		}
	case TransferSpeed:
		{
		n = llrint(fKey[c].toDouble());
		return n;
		}
	case TransferLoad:
		{
		q1 = fKey[c].left(fKey[c].find(","));
		q2 = fKey[c].mid(fKey[c].find(",")+1);
		bool ok1,ok2;
		n = toLongLong(q1, &ok1);
		m = toLongLong(q2, &ok2);
		if (ok1 && ok2)
		{
			o = 0;
			if (m == WSettings::Unlimited)
			{
				o = 0xFFFF0000 + n;
			}
			else if (m == 0)
			{
				return -1;
			}
			else if (n == 0)
			{
				o = m;
			}
			else
			{
				o = llrint((double) ( (double) n / (double) m * 10000.0f ));
				o = o * 100 + m;
			}
			return o;
		}
		else
		{
			return 0;
		}
		}
	case ConnectionSpeed:
		{
			bw = BandwidthToBytes(Q3ListViewItem::text(c));
			switch (bw)
			{
			case 75:
			case 300:
				return 1;
			case 14400:
				return 2;
			case 28800:
				return 3;
			case 33600:
				return 4;
			case 57600:
				return 5;
			case 64000:
				return 6;
			case 128000:
				return 7;
			case 256000:
				return 8;
			case 384000:
				return 9;
			case 512000:
				return 10;
			case 768000:
				return 11;
			case 1000000:
			case 1024000:
				return 12;
			case 1500000:
				return 13;
			case 4500000:
				return 14;
			case 155520000:
				return 15;
			case 622080000:
				return 16;
			default:
				return 0;
			}
		}
	default:
		{
			return -1;
		}
	}
}

QString
WUniListItem::text(int c) const
{
	QString postFix, result, q1, q2;
	double n;
	double o;
	int64 p, q;
	long lMod;
	uint32 secs, min, hours;
	bool ok;

#ifdef DEBUG2
	WString wres;
#endif

	switch (UColumnType[c])
	{
	case String_NoCase_Stripped:
	case String_Cased_Stripped:
		{
		return StripURL(Q3ListViewItem::text(c).stripWhiteSpace());
		}
	case Percentage:
		{
		return Q3ListViewItem::text(c)+" %";
		}
	case Size:
		{
		result = fKey[c];
		n = result.toDouble(&ok);
		postFix = qApp->translate( "WUniListItem", "B" );
		if (ok)
		{
			if (n >= 1024.0f)	// > 1 kB?
			{
				n /= 1024.0f;

				if (n >= 1024.0f)	// > 1 MB?
				{
					n /= 1024.0f;

					if (n >= 1024.0f)	// > 1 GB?
					{
						n /= 1024.0f;

						if (n >= 1024.0f) // > 1 TB?
						{
							n /= 1024.f;
							postFix = qApp->translate( "WUniListItem", "TB" );
						}
						else
							postFix = qApp->translate( "WUniListItem", "GB" );
					}
					else
						postFix = qApp->translate( "WUniListItem", "MB" );
				}
				else
					postFix = qApp->translate( "WUniListItem", "kB" );	// we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB

			}

		}
		else
			n = 0.0f;

		if (n != 0.0f)
		{
			result.sprintf("%.2f ", n);
			result += postFix;
#ifdef DEBUG2
			wres = result;
			PRINT2("UListView::text : %S\n", wres.getBuffer());
#endif
		}
		else
		{
			result = QString::null;
		}

		return result;
		}
	case TransferSpeed:
		{
		result = Q3ListViewItem::text(c);
		n = result.toDouble(&ok);
		postFix = qApp->translate( "WUniListItem", "B/s" );
		if (ok)
		{
			if (n >= 1024.0f)	// > 1 kB?
			{
				n /= 1024.0f;

				if (n >= 1024.0f)	// > 1 MB?
				{
					n /= 1024.0f;

					if (n >= 1024.0f)	// > 1 GB?
					{
						n /= 1024.0f;

						if (n >= 1024.0f) // > 1 TB?
						{
							n /= 1024.0f;
							postFix = qApp->translate( "WUniListItem", "TB/s" );
						}
						else
							postFix = qApp->translate( "WUniListItem", "GB/s" );
					}
					else
					postFix = qApp->translate( "WUniListItem", "MB/s" );
				}
				else
					postFix = qApp->translate( "WUniListItem", "kB/s" );	// we're in kilobytes now, <postmaster@raasu.org> 20021024 KB -> kB

			}
		}
		else
			n = 0.0f;

		if (n != 0.0f)
		{
			result.sprintf("%.2f ", n);
			result += postFix;
		}
		else
		{
			result = QString::null;
		}

		return result;
		}

	case TransferLoad:
		{
		q1 = fKey[c].left(fKey[c].find(","));
		q2 = fKey[c].mid(fKey[c].find(",")+1);
		bool ok1,ok2;
		p = toLongLong(q1, &ok1);
		q = toLongLong(q2, &ok2);
		if (ok1 && ok2)
		{
			o = 0.0f;
			if (q == WSettings::Unlimited)
			{
				QString tmp = fromLongLong(p);
				result = "(" + tmp + "/-)";
				return result;
			}
			if (q == 0)
				return QString::null;
			if (p == 0)
				o = 0.0f;
			else
				o = (double) ( (double) p / (double) q * 100.0f );
			QString qp = fromLongLong(p);
			QString qq = fromLongLong(q);
			QString qo; qo.sprintf("%3.2f %%", o);
			result = "(" + qp + "/" + qq +") " + qo;
		}
		return result;
		}
	case Date:
		{
		lMod = fKey[c].toLong();
		result = QString::fromLocal8Bit( ctime((const time_t *)&lMod) );
		result.truncate(result.length() - 1);
		return result;
		}
	case Time:
		{
		lMod = fKey[c].toLong();
		secs = lMod;
		min = secs / 60; secs = secs % 60;
		hours = min / 60; min = min % 60;

		if (lMod > 0)
		{
			result.sprintf("%lu:%.2lu:%.2lu", hours, min, secs);
		}
		else
		{
			// Don't show 0:00:00
			result = QString::null;
		}

		return result;
		}
	case ConnectionSpeed:
		{
			if (fKey[c].find(",") > 0)	// Strip 'hidden' bps value off
			{
				return fKey[c].left( fKey[c].find(",") );
			}
			else
			{
				return fKey[c];
			}
		}
	default:
		{
		return Q3ListViewItem::text(c);
		}
	}
}

void
WUniListItem::setColumnType(int c, WUniListItem::ColumnType ct)
{
	UColumnType[c] = ct;
}

WUniListItem::ColumnType
WUniListItem::columnType(int c) const
{
	return UColumnType[c];
}

// set/get user colors
void
WUniListItem::setRowBaseColor(int i, const QColor & color)
{
	RowBaseColor[i] = color;
}

QColor
WUniListItem::rowBaseColor(int i) const
{
	return RowBaseColor[i];
}

void
WUniListItem::setRowTextColor(int i, const QColor & color)
{
	RowTextColor[i] = color;
}

QColor
WUniListItem::rowTextColor(int i) const
{
	return RowTextColor[i];
}

void
WUniListItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int w, int alignment)
{
	Q3ListViewItem::paintCell(p, cg, column, w, alignment);
}

void
WUniListView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
         event->acceptProposedAction();
}

void
WUniListView::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> urls = event->mimeData()->urls();
		QList<QUrl>::iterator it = urls.begin();
		QPoint p = event->pos();
		QPoint q = viewport()->mapFromParent(p);
		Q3ListViewItem *li = itemAt(q);
		while (it != urls.end())
		{
			QString filename = (*it++).toLocalFile();
			QImageReader reader(filename);
			if (reader.canRead())
			{
				QImage img;
				if (reader.read(&img))
				{
#ifdef _DEBUG
					WString wfile(filename);
					WString wuser(li->text(1));
					PRINT("Sending picture \"%S\" to \"%S\"...\n", wfile.getBuffer(), wuser.getBuffer());
#endif
					gWin->SendPicture(li->text(1), filename);
				}
			}
		}
	}
}
