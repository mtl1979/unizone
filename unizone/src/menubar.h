#ifndef MENUBAR_H
#define MENUBAR_H

#pragma warning(disable: 4786)

#include <qmenubar.h>
#include <qpopupmenu.h>

class MenuBar : public Qt
{
public:
	MenuBar(QWidget * parent = NULL);
	~MenuBar();

	QPopupMenu * File() const { return fFile; }
	QPopupMenu * Edit() const { return fEdit; }
	QPopupMenu * Help() const { return fHelp; }

	QMenuBar * Bar() const { return fBar; }

private:
	/* Menu bar */
	QMenuBar * fBar;

	/* File menu */
	QPopupMenu * fFile;

	/* Edit menu */
	QPopupMenu * fEdit;

	/* Help menu */
	QPopupMenu * fHelp;
};

#endif


