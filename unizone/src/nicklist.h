// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef NICKLIST_H
#define NICKLIST_H

#include "platform.h"
#include "ulistview.h"
#include "nicklist.h"

#include <qlistview.h>

class WNickListItem : public WUniListItem
{
public:
enum 
{
	Name,
	ID,
	Status,
	Files,
	Connection,
	Load,
	Client,
	NumColumns
};

	WNickListItem(QListView * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}
	
	WNickListItem(QListViewItem * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}
	
	WNickListItem(QListView * parent, QListViewItem * after) 
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WNickListItem(QListViewItem * parent, QListViewItem * after)
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WNickListItem(QListView * parent, QString a, QString b = QString::null,
				QString c = QString::null, QString d = QString::null,
				QString e = QString::null, QString f = QString::null,
				QString g = QString::null, QString h = QString::null );

	// if more constructors are needed, they will be added later

	virtual void paintCell(QPainter *, const QColorGroup & cg, int column, int w,
							int alignment);
	
};

#endif
