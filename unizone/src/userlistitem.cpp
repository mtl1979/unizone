#include "userlistitem.h"
#include "global.h"			// defines gWin
#include "settings.h"		// defines WSettings and gWin->fSettings's type

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

WUserListItem::WUserListItem(
							 QListView * parent, 
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
		
	setRowBaseColor(0, Qt::white);			setRowTextColor(0, Qt::black);
	setRowBaseColor(1, Qt::green);			setRowTextColor(1, Qt::black);
	setRowBaseColor(2, Qt::yellow);			setRowTextColor(2, Qt::black);
	setRowBaseColor(3, Qt::darkYellow);		setRowTextColor(3, Qt::white);
	setRowBaseColor(4, Qt::red);			setRowTextColor(4, Qt::black);
	setRowBaseColor(5, Qt::darkRed);		setRowTextColor(5, Qt::white);
	setRowBaseColor(6, Qt::blue);			setRowTextColor(6, Qt::white);
}

void
WUserListItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int w, int alignment)
{
	
	_cg = cg;
	
	if (gWin->fSettings->GetMultiColor() == true)
	{
		long tx = WUniListItem::item(Load);
		
		if ((column == 0) || (column == 5))
		{
			if (tx > 0xFFFEFFFF && tx != -1)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(6));	// Infinite slots 
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(6));	//  
			}
			else if (tx > 999999 && tx < 0xFFFF0000)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(5));	// Full
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(5));	// 
			}
			else if (tx > 749999 && tx < 1000000)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(4));	// 3/4		- Full
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(4));	// 
			}
			else if (tx > 499999 && tx < 750000)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(3));	// 1/2		- 3/4 Full
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(3));	// 
			}
			else if (tx > 249999 && tx < 500000)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(2));	// 1/4		- 1/2 Full
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(2));	// 
			}
			else if (tx > -1 && tx < 250000)
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(1));	// Empty	- 1/4 Full
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(1));	// 
			}
			else
			{
				_cg.setColor(QColorGroup::ColorRole::Base, rowBaseColor(0));	// default
				_cg.setColor(QColorGroup::ColorRole::Text, rowTextColor(0));	// 
			}
		}
		else if (column == 3)
		{
			if (fFire)															// Firewalled?
				_cg.setColor(QColorGroup::ColorRole::Text, rowBaseColor(4));
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
	
	p->fillRect( 0, 0, w, height(), _cg.base() );
	
	WNickListItem::paintCell(p, _cg, column, w, alignment);
}
