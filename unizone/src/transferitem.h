// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include "ulistview.h"

#include <qlistview.h>

class WTransferItem : public WUniListItem
{
public:
	enum
	{
		Status,
		Filename,
		Received,
		Total,
		Rate,
		ETA,
		Elapsed,
		User,
		Index,
		QR,
		NumColumns
	};

	WTransferItem(Q3ListView * parent)
		: WUniListItem(parent)
	{
		// empty
	}

	WTransferItem(Q3ListViewItem * parent)
		: WUniListItem(parent)
	{
		// empty
	}
	
	WTransferItem(Q3ListView * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}
	
	WTransferItem(Q3ListViewItem * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}
	
	WTransferItem(Q3ListView * parent, QString a,
				QString b = QString::null, QString c = QString::null,
				QString d = QString::null, QString e = QString::null,
				QString f = QString::null, QString g = QString::null,
				QString h = QString::null, QString i = QString::null,
				QString j = QString::null);

	// if more constructors are needed, they will be added later

	
};

#endif
