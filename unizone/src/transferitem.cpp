// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include "transferitem.h"

WTransferItem::WTransferItem(
							 QListView * parent, 
							 QString a, QString b, QString c, QString d,
							 QString e, QString f, QString g, QString h, QString i
							 )
							 : WUniListItem(parent, a, b, c, d, e, f, g, h, i) 
{
	setText(Status, a);
	setText(Filename, b);
	setText(Received, c);
	setText(Total, d);
	setText(Rate, e);
	setText(ETA, f);
	setText(User, g);
	setText(Index, h);
	setText(QR, i);
	
	setColumnType(Status, String_Cased);
	setColumnType(Filename, String_Cased);
	setColumnType(Received, Size);
	setColumnType(Total, Size);
	setColumnType(Rate, TransferSpeed);
	setColumnType(ETA, Time);
	setColumnType(User, String_NoCase_Stripped);
	setColumnType(Index, String_NoCase);
	setColumnType(QR, Number);
}
