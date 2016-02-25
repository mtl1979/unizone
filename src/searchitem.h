#ifndef SEARCHITEM_H
#define SEARCHITEM_H

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
		Info,
		NumColumns
	};

	WSearchListItem(Q3ListView * parent)
		: WUniListItem(parent)
	{
		// empty
	}

	WSearchListItem(Q3ListViewItem * parent)
		: WUniListItem(parent)
	{
		// empty
	}

	WSearchListItem(Q3ListView * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}

	WSearchListItem(Q3ListViewItem * parent, Q3ListViewItem * after)
		: WUniListItem(parent, after)
	{
		// empty
	}

	WSearchListItem(
		Q3ListView * parent, QString a,
		QString b = QString::null, QString c = QString::null, QString d = QString::null,
		QString e = QString::null, QString f = QString::null, QString g = QString::null
		);
	// if more constructors are needed, they will be added later
	virtual QString text(int c) const;

};

#endif	// SEARCHITEM_H
