#pragma warning(disable: 4786)
#include "checkgroup.h"


QCheckGroup::QCheckGroup(QMenuBar * bar, bool ex)
	: fEx(ex), fBar(bar)
{
}

QCheckGroup::~QCheckGroup()
{
}

void
QCheckGroup::Add(int itemID, const QString & text)
{
	QCheckItem item;
	item.first = text;
	item.second = itemID;

	fMap.insert(item);
}

void
QCheckGroup::Remove(const QString & text)
{
	QCheckMap::iterator it;

	it = fMap.find(text);
	if (it != fMap.end())
	{
		fBar->setItemChecked((*it).second, false);
		fMap.erase(it);
	}
}

void
QCheckGroup::Check(const QString & text)
{
	QCheckMap::iterator it = fMap.find(text);

	if (it != fMap.end())
	{
		if (fEx)
		{
			for (QCheckMap::iterator scan = fMap.begin(); scan != fMap.end(); scan++)
				fBar->setItemChecked((*scan).second, false);
		}
		fBar->setItemChecked((*it).second, true);
	}
}

void
QCheckGroup::Toggle(const QString & text)
{
	if (fEx)	// if exclusive
	{
		// check/uncheck the item
		if (IsChecked(text))
			Uncheck(text);
		else
			Check(text);
		return;
	}

	// else...
	QCheckMap::iterator it = fMap.find(text);

	if (it != fMap.end())
		fBar->setItemChecked((*it).second, !fBar->isItemChecked((*it).second));
}

void
QCheckGroup::Uncheck(const QString & text)
{
	if (fEx)
		return;

	QCheckMap::iterator it = fMap.find(text);

	if (it != fMap.end())
		fBar->setItemChecked((*it).second, false);
}

bool
QCheckGroup::IsChecked(const QString & text)
{
	QCheckMap::iterator it = fMap.find(text);

	if (it != fMap.end())
		return fBar->isItemChecked((*it).second);

	return false;
}
