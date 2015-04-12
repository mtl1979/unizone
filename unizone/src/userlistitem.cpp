// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include <qpainter.h>

#include "userlistitem.h"
#include "global.h"			// defines gWin
#include "settings.h"		// defines WSettings and gWin->fSettings's type

WUserListItem::WUserListItem(
							 Q3ListView * parent,
							 QString a, QString b, QString c, QString d,
							 QString e, QString f, QString g, QString h
							 )
							 : WNickListItem(parent, a, b, c, d, e, f, g, h)
{
	setText(Name,a);
	setText(ID,b);
	setText(Status,c);
	setText(Files,d);
	setText(Connection,e);
	setText(Load,f);
	setText(Client,g);
	setText(HostOS,h);
	
	setRowBaseColor(0, Qt::white);
	setRowBaseColor(1, Qt::green);
	setRowBaseColor(2, Qt::yellow);
	setRowBaseColor(3, Qt::darkYellow);
	setRowBaseColor(4, Qt::red);
	setRowBaseColor(5, Qt::darkRed);
	setRowBaseColor(6, Qt::blue);
	
	setRowTextColor(0, Qt::black);
	setRowTextColor(1, Qt::black);
	setRowTextColor(2, Qt::black);
	setRowTextColor(3, Qt::white);
	setRowTextColor(4, Qt::black);
	setRowTextColor(5, Qt::white);
	setRowTextColor(6, Qt::white);
}

void
WUserListItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int w, int alignment)
{
	QColorGroup _cg = cg;
	
	bool mc = false;
	if (gWin->fSettings)
	{
		mc = gWin->fSettings->GetMultiColor();
	}
	
	if (mc == true)
	{
		int64 tx = item(Load);
		
		if ((column == 0) || (column == 5))
		{
			if (tx > 999999)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(5));	// Full
				_cg.setColor(QColorGroup::Text, rowTextColor(5));	//
			}
			else if (tx > 749999 && tx < 1000000)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(4));	// 3/4		- Full
				_cg.setColor(QColorGroup::Text, rowTextColor(4));	//
			}
			else if (tx > 499999 && tx < 750000)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(3));	// 1/2		- 3/4 Full
				_cg.setColor(QColorGroup::Text, rowTextColor(3));	//
			}
			else if (tx > 249999 && tx < 500000)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(2));	// 1/4		- 1/2 Full
				_cg.setColor(QColorGroup::Text, rowTextColor(2));	//
			}
			else if (tx > -1 && tx < 250000)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(1));	// Empty - 1/4 Full
				_cg.setColor(QColorGroup::Text, rowTextColor(1));	//
			}
			else if (tx == -1)
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(0));	// default
				_cg.setColor(QColorGroup::Text, rowTextColor(0));	//
			}
			else // tx < -1
			{
				_cg.setColor(QColorGroup::Base, rowBaseColor(6));	// Infinite slots
				_cg.setColor(QColorGroup::Text, rowTextColor(6));	//
			}
			
		}
		else if (column == 3)
		{
			if (fFire)															// Firewalled?
				_cg.setColor(QColorGroup::Text, rowBaseColor(4));
		}
	}
	else
	{
		if (fFire)
		{
			QFont font = p->font();
			font.setBold(true);
			p->setFont(font);
		}
	}
	WNickListItem::paintCell(p, _cg, column, w, alignment);
}
