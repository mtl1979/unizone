#include "searchitem.h"

#include <qregexp.h>

WSearchListItem::WSearchListItem(	
								 QListView * parent,
								 QString a, QString b, QString c, QString d,
								 QString e, QString f, QString g, QString h 
								 )
								 : WUniListItem(parent, a, b, c, d, e, f, g, h)
{
	setText(FileName, a);
	setText(FileSize, b);
	setText(FileType, c);
	setText(Modified, d);
	setText(Path, e);
	setText(User, f);
	
	setColumnType(FileName, String_NoCase);
	setColumnType(FileSize, Size);
	setColumnType(FileType, String_NoCase);
	setColumnType(Modified, Date);
	setColumnType(User, String_NoCase_Stripped);
}

QString 
WSearchListItem::text(int c) const
{
	QString tmp = WUniListItem::text(c);
	switch (columnType(c))
	{
	case String_NoCase:
	case String_NoCase_Stripped:
	case String_Cased:
	case String_Cased_Stripped:
		tmp.replace(QRegExp("\r\n"), " ");
		tmp.replace(QRegExp("\r"), " ");
		tmp.replace(QRegExp("\n"), " ");
	};
	return tmp;
}
