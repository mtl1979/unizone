#ifndef CHECKGROUP_H
#define CHECKGROUP_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qmenubar.h>

#include <map>
using std::map;
using std::pair;

typedef pair<QString, int> QCheckItem;
typedef map<QString, int> QCheckMap;

class QCheckGroup
{
public:
	QCheckGroup(QMenuBar * bar, bool exclusive = false);
	~QCheckGroup();

	/* Add an item to the group */
	void Add(int itemID, const QString & text);
	/* Remove an item from the group, by its associated text. */
	void Remove(const QString & text);


	/* Check an item, by its associated text. */
	void Check(const QString & text);
	/* Check to see if an item is checked */
	bool IsChecked(const QString & text);
	/* Uncheck an item. */
	void Uncheck(const QString & text);
	/* Toggle an item */
	void Toggle(const QString & text);

private:
	bool fEx;
	QMenuBar * fBar;
	QCheckMap fMap;
};


#endif


