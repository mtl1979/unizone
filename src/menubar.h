#ifndef MENUBAR_H
#define MENUBAR_H

#include <qmenubar.h>
#include <q3popupmenu.h>

class MenuBar : public QMenuBar
{
	Q_OBJECT
public:
	MenuBar(QWidget * parent = NULL);
	~MenuBar();

	Q3PopupMenu * File() const { return fFile; }
	Q3PopupMenu * Edit() const { return fEdit; }
	Q3PopupMenu * Help() const { return fHelp; }
	Q3PopupMenu * Windows() const { return fWindows; }

private:

	/* File menu */
	Q3PopupMenu * fFile;

	/* Edit menu */
	Q3PopupMenu * fEdit;

	/* Windows menu */
	Q3PopupMenu * fWindows;

	/* Help menu */
	Q3PopupMenu * fHelp;
};

#endif
