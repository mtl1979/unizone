#include "menubar.h"

#include <qapplication.h>
#include <qaccel.h>

MenuBar::MenuBar(QWidget * parent) : QMenuBar(parent)
{
	/* create file menu */
	fFile = new QPopupMenu(this);
	CHECK_PTR(fFile);
	fFile->insertItem(tr("&Open"), parent, SLOT(Load()), QAccel::stringToKey(tr("CTRL+O")));
	fFile->insertItem(tr("&Close"), parent, SLOT(ClearImage()));

	fFile->insertSeparator();

	fFile->insertItem(tr("E&xit"), parent, SLOT(Exit()), QAccel::stringToKey(tr("ALT+X")));

	/* Insert into menubar */
	insertItem(tr("&File"), fFile);
}

MenuBar::~MenuBar()
{
}

