// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include "ulistview.h"
#include "nicklist.h"

#include <q3listview.h>
#include <qpalette.h>

class WUserListItem : public WNickListItem
{
public:

	WUserListItem(Q3ListView * parent) 
		: WNickListItem(parent) 
	{
		// empty
	}
	
	WUserListItem(Q3ListViewItem * parent) 
		: WNickListItem(parent) 
	{
		// empty
	}
	
	WUserListItem(Q3ListView * parent, Q3ListViewItem * after) 
		: WNickListItem(parent, after) 
	{
		// empty
	}
	
	WUserListItem(Q3ListViewItem * parent, Q3ListViewItem * after)
		: WNickListItem(parent, after) 
	{
		// empty
	}
	
	WUserListItem(Q3ListView * parent, QString a, QString b = QString::null,
				QString c = QString::null, QString d = QString::null,
				QString e = QString::null, QString f = QString::null,
				QString g = QString::null, QString h = QString::null );

	// if more constructors are needed, they will be added later

	virtual void paintCell(QPainter *, const QColorGroup & cg, int column, int w,
							int alignment);
	

	void SetFirewalled(bool b) 
	{ 
		fFire = b; 
	}
	
	bool Firewalled() const
	{ 
		return fFire; 
	}

private:
	bool fFire;
	QColorGroup _cg;
};

#endif
