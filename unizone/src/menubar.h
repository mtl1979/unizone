#ifndef MENUBAR_H
#define MENUBAR_H

#pragma warning(disable: 4786)

#include <qmenubar.h>
#include <qpopupmenu.h>
#include "checkgroup.h"

class MenuBar : public Qt
{
public:
	MenuBar(QWidget * parent = NULL);
	~MenuBar();

	QPopupMenu * Edit() const { return fEdit; }

	QMenuBar * Bar() const { return fBar; }

private:
	QMenuBar * fBar;

	/* File menu */
	QPopupMenu * fFile;

	QPopupMenu * fEdit;
};

#endif


