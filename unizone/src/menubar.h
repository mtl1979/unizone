#ifndef MENUBAR_H
#define MENUBAR_H

#pragma warning(disable: 4786)

#include <qmenubar.h>
#include <qpopupmenu.h>

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget * handler, QWidget * parent = NULL);
	~MenuBar();

	QPopupMenu * File() const { return fFile; }
	QPopupMenu * Edit() const { return fEdit; }
	QPopupMenu * Help() const { return fHelp; }
	QPopupMenu * Windows() const { return fWindows; }

//	QMenuBar * Bar() const { return fBar; }

private:
	/* Menu bar */
//	QMenuBar * fBar;

	/* File menu */
	QPopupMenu * fFile;

	/* Edit menu */
	QPopupMenu * fEdit;

	/* Windows menu */
	QPopupMenu * fWindows;

	/* Help menu */
	QPopupMenu * fHelp;
};

#endif


