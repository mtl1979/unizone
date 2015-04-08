// Universal List View class (C) 2002 FieldNet Association / Team UniShare
// Released under Lesser GPL as in LGPL.TXT in source root folder

#ifndef ULISTITEM_H
#define ULISTITEM_H

#include <qpalette.h>
#include <q3listview.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "utypes.h"

class WUniListItem : public Q3ListViewItem
{
public:

	// Column Type constants
	enum ColumnType
	{
		Generic,					// Generic string
		Number,						// Normal numerical value
		Percentage,					// Normal numerical value with percent sign
		Size,						// File Size (B, kB, MB, GB)
		String_NoCase,				// Case is unsignificant
		String_NoCase_Stripped,		// - URLs will be stripped
		String_Cased,				// Case is significant
		String_Cased_Stripped,		// - URLs will be stripped
		TransferLoad,				// Transfer Load in form 'Current,Maximum'
		Date,						// Time or Date
		TransferSpeed,				// Transfer Speed, just like Size, but per second
		ConnectionSpeed,			// Connection Speed
		Time,						// Time expressed in seconds
		Invalid						// 
	} ;

#define NUM_ROW_COLORS	10
#define NUM_COLUMNS		10
	
	WUniListItem(Q3ListView * parent) 
		: Q3ListViewItem(parent) 
	{
		// empty
	}

	WUniListItem(Q3ListViewItem * parent) 
		: Q3ListViewItem(parent) 
	{
		// empty
	}

	WUniListItem(Q3ListView * parent, Q3ListViewItem * after) 
		: Q3ListViewItem(parent, after) 
	{
		// empty
	}

	WUniListItem(Q3ListViewItem * parent, Q3ListViewItem * after) 
		: Q3ListViewItem(parent, after) 
	{
		// empty
	}

	WUniListItem(Q3ListView * parent, QString a, 
		QString b = QString::null, QString c = QString::null, 
		QString d = QString::null, QString e = QString::null, 
		QString f = QString::null, QString g = QString::null, 
		QString h = QString::null, QString i = QString::null,
		QString j = QString::null);
	
	// if more constructors are needed, they will be added later
	
	
	virtual void setText(int col, const QString & text);
	virtual QString text(int c) const;
	
	// for case insensitive sorting
	virtual QString key(int c, bool asc) const;

	// returns numeric equivalent of key()
	virtual int64 item(int c);

	// set/get user colors
	virtual void setRowBaseColor(int i, const QColor & color); 
	virtual void setRowTextColor(int i, const QColor & color); 
	virtual QColor rowBaseColor(int i) const;
	virtual QColor rowTextColor(int i) const;
		
	// set column type for sorting
	virtual void setColumnType(int c, WUniListItem::ColumnType ct);
	virtual WUniListItem::ColumnType columnType(int c) const;

	virtual void paintCell(QPainter *, const QColorGroup & cg, int column, int w,
		int alignment);

	
private:
	QString fKey[NUM_COLUMNS];
	WUniListItem::ColumnType UColumnType[NUM_COLUMNS];
	QColor RowBaseColor[NUM_ROW_COLORS];
	QColor RowTextColor[NUM_ROW_COLORS];

};

class WUniListView : public Q3ListView
{
	Q_OBJECT
public:
	WUniListView( QWidget * parent, const char * name, Qt::WindowFlags f ) 
		: Q3ListView( parent, name, f )
	{
		if (!name)
			setName("WUniListView");
	}
	
	WUniListView( QWidget * parent = 0, const char * name = 0 ) 
		: Q3ListView( parent, name )
	{
		if (!name)
			setName("WUniListView");
	}
	
	virtual void setSorting( int column, bool ascending = TRUE )
	{
		_sortColumn = column;
		_sortAscending = ascending;
		Q3ListView::setSorting(column, ascending);
	}
	
	virtual int sortColumn() 
	{ 
		return _sortColumn; 
	}
	
	virtual bool sortAscending() 
	{ 
		return _sortAscending; 
	}
	
protected:
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
private:
	int _sortColumn;
	bool _sortAscending;
};

#endif
