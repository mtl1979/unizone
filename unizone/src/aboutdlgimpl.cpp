#include <stdio.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "aboutdlgimpl.h"
#include "ui_aboutdlg.h"
#include "version.h"

/*
 *  Constructs a AboutDlg which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AboutDlg::AboutDlg( QWidget* parent,  const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
	Ui_AboutDlgBase *ui = new Ui_AboutDlgBase();
	ui->setupUi(this);

	if ( !name )
		setName("AboutDlg");
	connect(ui->buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
	QString about = tr("Unizone (English)");
	ui->titleLabel->setProperty("text", about);
	QString version = tr("Version %1").arg(WinShareVersionString());
	ui->versionLabel->setProperty("text", version);
	setCaption(tr("About Unizone (English)"));
	ui->TextLabel2_3_2->setText( tr( "Unizone is Copyright (C) %1 by Mika T. Lindqvist" ).arg(GetUnizoneYears()) );
}


/*
 *  Destroys the object and frees any allocated resources
 */
AboutDlg::~AboutDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

