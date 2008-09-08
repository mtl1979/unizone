#include "menubar.h"

#include <qapplication.h>
#include <q3accel.h>
#include <Q3PopupMenu>

MenuBar::MenuBar(QWidget * parent) : QMenuBar(parent)
{
	/* create file menu */
	fFile = new Q3PopupMenu(this);
	Q_CHECK_PTR(fFile);
	fFile->insertItem(tr("&Open"), parent, SLOT(Load()), Q3Accel::stringToKey(tr("CTRL+O")));
	fFile->insertItem(tr("&Close"), parent, SLOT(ClearImage()));

	fFile->insertSeparator();

	fFile->insertItem(tr("E&xit"), parent, SLOT(Exit()), Q3Accel::stringToKey(tr("ALT+X")));

	/* Insert into menubar */
	insertItem(tr("&File"), fFile);
}

MenuBar::~MenuBar()
{
}

