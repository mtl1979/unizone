// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include "platform.h"
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
		User,
		Index,
		NumColumns
	};

	WTransferItem(QListView * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}

	WTransferItem(QListViewItem * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}
	
	WTransferItem(QListView * parent, QListViewItem * after) 
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WTransferItem(QListViewItem * parent, QListViewItem * after)
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WTransferItem(QListView * parent, QString a, QString b = QString::null,
				QString c = QString::null, QString d = QString::null,
				QString e = QString::null, QString f = QString::null,
				QString g = QString::null, QString h = QString::null );

	// if more constructors are needed, they will be added later

	
};

#endif
