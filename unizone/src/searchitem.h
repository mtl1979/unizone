#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include "platform.h"
#include "ulistview.h"

#include <qlistview.h>

class WSearchListItem : public WUniListItem
{
public:
	enum 
	{
		FileName,
		FileSize,
		FileType,
		Modified,
		Path,
		User,
		NumColumns
	};

	WSearchListItem(QListView * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}
	
	WSearchListItem(QListViewItem * parent) 
		: WUniListItem(parent) 
	{
		// empty
	}
	
	WSearchListItem(QListView * parent, QListViewItem * after) 
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WSearchListItem(QListViewItem * parent, QListViewItem * after) 
		: WUniListItem(parent, after) 
	{
		// empty
	}
	
	WSearchListItem(
		QListView * parent, QString a, 
		QString b = QString::null, QString c = QString::null, QString d = QString::null,
		QString e = QString::null, QString f = QString::null, QString g = QString::null, 
		QString h = QString::null 
		);
	// if more constructors are needed, they will be added later

};

#endif	// SEARCHITEM_H
