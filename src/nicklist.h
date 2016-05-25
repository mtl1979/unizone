// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef NICKLIST_H
#define NICKLIST_H

#include "ulistview.h"

#include <q3listview.h>
#include <qpalette.h>

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
		HostOS,
		NumColumns
	};

	WNickListItem(Q3ListView * parent)
		: WUniListItem(parent)
	{
		// empty
	}

	WNickListItem(Q3ListViewItem * parent)
		: WUniListItem(parent)
	{
		// empty
	}

	WNickListItem(Q3ListView * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}

	WNickListItem(Q3ListViewItem * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}

	WNickListItem(Q3ListView * parent, QString a, QString b = QString::null,
		QString c = QString::null, QString d = QString::null,
		QString e = QString::null, QString f = QString::null,
		QString g = QString::null, QString h = QString::null );

	// if more constructors are needed, they will be added later

	virtual void paintCell(QPainter * p, const QColorGroup & cg, int column, int w,
		int alignment);

};

#endif
