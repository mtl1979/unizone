#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "aboutdlgimpl.h"
#include "global.h"

#include <stdio.h>
#include <qlabel.h>
#include <qpushbutton.h>


/* 
 *  Constructs a AboutDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AboutDlg::AboutDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : AboutDlgBase( parent, name, modal, fl )
{
	if ( !name ) 
		setName("AboutDlg");
	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	QString about = tr("Unizone (English)\nVersion %1").arg(WinShareVersionString());
	titleLabel->setProperty("text", about);
	setCaption(tr("About Unizone (English)"));
}


/*  
 *  Destroys the object and frees any allocated resources
 */
AboutDlg::~AboutDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

