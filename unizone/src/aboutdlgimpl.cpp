#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "aboutdlgimpl.h"
#include "version.h"
#include "lang.h"			// <postmaster@raasu.org> 20020924

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
	char about[100];
	sprintf(about, NAME "\nVersion %s", WinShareVersionString()); // <postmaster@raasu.org> 20020926 Moved 'Version' to line 2
	titleLabel->setProperty("text", tr(about));
	setCaption(tr("About " NAME));
}


/*  
 *  Destroys the object and frees any allocated resources
 */
AboutDlg::~AboutDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

