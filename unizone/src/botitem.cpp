// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#include "botitem.h"
#include "nicklist.h"

WBotItem::WBotItem(	
				   QListView * parent,
				   QString a, QString b, QString c, QString d,
				   QString e, QString f, QString g, QString h 
				   )
				   : WNickListItem(parent, a, b, c, d, e, f, g, h) 
{
	// empty
}

void
WBotItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int w, int alignment)
{
	QFont font = p->font();
	font.setItalic(true);
	p->setFont(font);

	WNickListItem::paintCell(p, cg, column, w, alignment);
}
