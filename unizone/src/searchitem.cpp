#include "searchitem.h"

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
