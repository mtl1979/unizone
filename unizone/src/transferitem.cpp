// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include "transferitem.h"

WTransferItem::WTransferItem(
							 QListView * parent, 
							 QString a, QString b, QString c, QString d,
							 QString e, QString f, QString g, QString h
							 )
							 : WUniListItem(parent, a, b, c, d, e, f, g, h) 
{
	setText(Status, a);
	setText(Filename, b);
	setText(Received, c);
	setText(Total, d);
	setText(Rate, e);
	setText(ETA, f);
	setText(User, g);
	
	setColumnType(Status, String_Cased);
	setColumnType(Filename, String_Cased);
	setColumnType(Received, Size);
	setColumnType(Total, Size);
	setColumnType(Rate, TransferSpeed);
	setColumnType(ETA, Time);
	setColumnType(User, String_NoCase_Stripped);
	
}
