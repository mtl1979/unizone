#include "prefsimpl.h"
#include "prefs.h"
#include "global.h"
#include "settings.h"
#include "colors.h"
#include "debugimpl.h"

#include <qmotifstyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qcdestyle.h>
#include <qinterlacestyle.h>
#include <qmotifplusstyle.h>
#include <qsgistyle.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qlineedit.h>

const char * kColorDesc[11] = {	
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of your user name." ), 
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of other users' names." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of text sent by you and other users." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of \"System\"." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of the text in a ping response." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of \"Error\"." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of the text in error messages." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of private text." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of \"Action\"." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of URLs." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of your user name in text when someone says your name in the main chat." )
								};

const char * kSampleText[11] = {	
									QT_TRANSLATE_NOOP( "WPrefs", "Username" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Remote User" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Sample text" ),
									QT_TRANSLATE_NOOP( "WPrefs", "<b>System</b> " ),
									QT_TRANSLATE_NOOP( "WPrefs", "Sample text" ),
									QT_TRANSLATE_NOOP( "WPrefs", "<b>Error</b>" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Error text" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Private text" ),
									QT_TRANSLATE_NOOP( "WPrefs", "<b>Action</b>" ),
									QT_TRANSLATE_NOOP( "WPrefs", "<u>http://magep.com/</u>" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Username" )
								};

/*
 *  Constructs a prefs which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WPrefs::WPrefs( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : WPrefsBase( parent, name, modal, fl )
{
	if ( !name ) 
		setName( "WPrefs" );
	connect(fOK, SIGNAL(clicked()), this, SLOT(OK()));
	connect(fCancel, SIGNAL(clicked()), this, SLOT(Cancel()));
	connect(fStyleList, SIGNAL(highlighted(int)), this, SLOT(StyleSelected(int)));
	connect(fColorsList, SIGNAL(highlighted(int)), this, SLOT(ColorSelected(int)));
	connect(fChange, SIGNAL(clicked()), this, SLOT(ChangeColor()));
	connect(fAutoAway, SIGNAL(highlighted(int)), this, SLOT(AwaySelected(int)));

	fCurColorIndex = -1;
	// initialize the colors
	fColor[0] = WColors::LocalName;
	fColor[1] = WColors::RemoteName;
	fColor[2] = WColors::Text;
	fColor[3] = WColors::System;
	fColor[4] = WColors::Ping;
	fColor[5] = WColors::Error;
	fColor[6] = WColors::ErrorMsg;
	fColor[7] = WColors::PrivText;
	fColor[8] = WColors::Action;
	fColor[9] = WColors::URL;
	fColor[10] = WColors::NameSaid;

	
	fAutoUpdateServers->setChecked(gWin->fSettings->GetAutoUpdateServers());
	fNewVersions->setChecked(gWin->fSettings->GetCheckNewVersions());
	fLoginStartup->setChecked(gWin->fSettings->GetLoginOnStartup());
	fFireWalled->setChecked(gWin->fSettings->GetFirewalled());
	fBinkyNuke->setChecked(gWin->fSettings->GetBinkyNuke());
	fBlockDisconnected->setChecked(gWin->fSettings->GetBlockDisconnected());
	fAutoClear->setChecked(gWin->fSettings->GetAutoClear());
	fMultiColor->setChecked(gWin->fSettings->GetMultiColor());
	if (gWin->fSettings->GetConnection() != "?")
	{
		for (int i = 0; i < fBandwidth->count(); i++)
		{
			if (fBandwidth->text(i) == gWin->fSettings->GetConnection())
			{
				fBandwidth->setCurrentItem(i);
				break;
			}
		}
	}

	fTimeStamps->setChecked(gWin->fSettings->GetTimeStamps());
	fUserEvents->setChecked(gWin->fSettings->GetUserEvents());
	fUploads->setChecked(gWin->fSettings->GetUploads());
	fDownloads->setChecked(gWin->fSettings->GetDownloads());
	fChat->setChecked(gWin->fSettings->GetChat());
	fPrivate->setChecked(gWin->fSettings->GetPrivate());
	fInfo->setChecked(gWin->fSettings->GetInfo());
	fWarning->setChecked(gWin->fSettings->GetWarning());
	fError->setChecked(gWin->fSettings->GetError());
	fSounds->setChecked(gWin->fSettings->GetSounds());
	
	switch (gWin->fSettings->GetStyle())
	{
		case WinShareWindow::CDE:
			fStyleList->setCurrentItem(0);
			break;

		case WinShareWindow::Motif:
			fStyleList->setCurrentItem(1);
			break;

		case WinShareWindow::MotifPlus:
			fStyleList->setCurrentItem(2);

		case WinShareWindow::Platinum:
			fStyleList->setCurrentItem(3);
			break;

		case WinShareWindow::SGI:
			fStyleList->setCurrentItem(4);
			break;

		case WinShareWindow::WindowsStyle:
			fStyleList->setCurrentItem(5);
			break;
	}

	// init auto away
	fAutoAway->setCurrentItem(gWin->fSettings->GetAutoAway());

#ifdef WIN32
	// init flash flags
	if (gWin->fSettings->GetFlash() & WSettings::FlashMain)
		fFlashMain->setChecked(true);
	else
		fFlashMain->setChecked(false);

	if (gWin->fSettings->GetFlash() & WSettings::FlashPriv)
		fFlashPrivate->setChecked(true);
	else
		fFlashPrivate->setChecked(false);
#else
	// linux doesn't get this nifty feature
	fFlashMain->hide();
	fFlashPrivate->hide();
#endif

	switch (gWin->fSettings->GetMaxDownloads())
	{
		case WSettings::One:
			fMaxDL->setCurrentItem(0); break;
		case WSettings::Two:
			fMaxDL->setCurrentItem(1); break;
		case WSettings::Three:
			fMaxDL->setCurrentItem(2); break;
		case WSettings::Four:
			fMaxDL->setCurrentItem(3); break;
		case WSettings::Five:
			fMaxDL->setCurrentItem(4); break;
		case WSettings::Ten:
			fMaxDL->setCurrentItem(5); break;
		case WSettings::Fifteen:
			fMaxDL->setCurrentItem(6); break;
		case WSettings::Twenty:
			fMaxDL->setCurrentItem(7); break;
		case WSettings::Thirty:
			fMaxDL->setCurrentItem(8); break;
		case WSettings::Unlimited:
			fMaxDL->setCurrentItem(9); break;
	}

	switch (gWin->fSettings->GetMaxUploads())
	{
		case WSettings::One:
			fMaxUL->setCurrentItem(0); break;
		case WSettings::Two:
			fMaxUL->setCurrentItem(1); break;
		case WSettings::Three:
			fMaxUL->setCurrentItem(2); break;
		case WSettings::Four:
			fMaxUL->setCurrentItem(3); break;
		case WSettings::Five:
			fMaxUL->setCurrentItem(4); break;
		case WSettings::Ten:
			fMaxUL->setCurrentItem(5); break;
		case WSettings::Fifteen:
			fMaxUL->setCurrentItem(6); break;
		case WSettings::Twenty:
			fMaxUL->setCurrentItem(7); break;
		case WSettings::Thirty:
			fMaxUL->setCurrentItem(8); break;
		case WSettings::Unlimited:
			fMaxUL->setCurrentItem(9); break;
	}

	fFileSharingEnabled->setChecked(gWin->fSettings->GetSharingEnabled());
	printf("Setting font size\n");
	fFontSize->setValue(gWin->fSettings->GetFontSize());

#ifdef WIN32	// windows has a system based launcher
	fTabs->setTabEnabled(fURLLaunching, false);
#else
	fMailtoLauncher->setText(gWin->fSettings->GetMailLauncher());
	fHTTPLauncher->setText(gWin->fSettings->GetHTTPLauncher());
	fFTPLauncher->setText(gWin->fSettings->GetFTPLauncher());
	fDefaultLauncher->setText(gWin->fSettings->GetDefaultLauncher());
#endif

	fChatLimit->setCurrentItem(gWin->fSettings->GetChatLimit());
	fULLimit->setCurrentItem(gWin->fSettings->GetULLimit());
	fDLLimit->setCurrentItem(gWin->fSettings->GetDLLimit());
	fBLLimit->setCurrentItem(gWin->fSettings->GetBLLimit());

	switch (gWin->fSettings->GetPacketSize())
	{
	case 1:
		fPacketSize->setCurrentItem(0); break;
	case 2:
		fPacketSize->setCurrentItem(1); break;
	case 4:
		fPacketSize->setCurrentItem(2); break;
	case 8:
		fPacketSize->setCurrentItem(3); break;
	case 16:
		fPacketSize->setCurrentItem(4); break;
	case 32:
		fPacketSize->setCurrentItem(5); break;
	}
	fMinQueued->setCurrentItem( gWin->fSettings->GetMinQueued() );
	
	fLogging->setChecked(gWin->fSettings->GetLogging());

	fBasePort->setText( QString::number( gWin->fSettings->GetBasePort() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
WPrefs::~WPrefs()
{
    // no need to delete child widgets, Qt does it all for us
}

void
WPrefs::OK()
{
	int i;

	// save the colors
	gWin->fSettings->EmptyColorList();
	for (i = 0; i < 11; i++)
		gWin->fSettings->AddColorItem(fColor[i]);
	WColors::LocalName = fColor[0];
	WColors::RemoteName = fColor[1];
	WColors::Text = fColor[2];
	WColors::System = fColor[3];
	WColors::Ping = fColor[4];
	WColors::Error = fColor[5];
	WColors::ErrorMsg = fColor[6];
	WColors::PrivText = fColor[7];
	WColors::Action = fColor[8];
	WColors::URL = fColor[9];
	WColors::NameSaid = fColor[10];

	// save all the other stuff
	gWin->fSettings->SetAutoUpdateServers(fAutoUpdateServers->isChecked());
	gWin->fSettings->SetCheckNewVersions(fNewVersions->isChecked());
	gWin->fSettings->SetLoginOnStartup(fLoginStartup->isChecked());
	gWin->fSettings->SetConnection(fBandwidth->currentText());
	gWin->fSettings->SetFirewalled(fFireWalled->isChecked());
	gWin->fSettings->SetBinkyNuke(fBinkyNuke->isChecked());
	gWin->fSettings->SetBlockDisconnected(fBlockDisconnected->isChecked());
	gWin->fSettings->SetAutoClear(fAutoClear->isChecked());
	gWin->fSettings->SetMultiColor(fMultiColor->isChecked());
	gWin->fSettings->SetTimeStamps(fTimeStamps->isChecked());
	gWin->fSettings->SetUserEvents(fUserEvents->isChecked());
	gWin->fSettings->SetUploads(fUploads->isChecked());
	gWin->fSettings->SetDownloads(fDownloads->isChecked());
	gWin->fSettings->SetChat(fChat->isChecked());
	gWin->fSettings->SetPrivate(fPrivate->isChecked());
	gWin->fSettings->SetInfo(fInfo->isChecked());
	gWin->fSettings->SetWarning(fWarning->isChecked());
	gWin->fSettings->SetError(fError->isChecked());
	gWin->fSettings->SetSounds(fSounds->isChecked());

	// flash settings
	int flags = WSettings::FlashNone;
	if (fFlashMain->isChecked())
		flags |= WSettings::FlashMain;
	if (fFlashPrivate->isChecked())
		flags |= WSettings::FlashPriv;
	gWin->fSettings->SetFlash(flags);

	gWin->fSettings->SetSharingEnabled(fFileSharingEnabled->isChecked());

	switch (fMaxDL->currentItem())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gWin->fSettings->SetMaxDownloads(fMaxDL->currentItem() + 1); break;
		case 5:
			gWin->fSettings->SetMaxDownloads(WSettings::Ten); break;
		case 6:
			gWin->fSettings->SetMaxDownloads(WSettings::Fifteen); break;
		case 7:
			gWin->fSettings->SetMaxDownloads(WSettings::Twenty); break;
		case 8:
			gWin->fSettings->SetMaxDownloads(WSettings::Thirty); break;
		case 9:
			gWin->fSettings->SetMaxDownloads(WSettings::Unlimited); break;
	}

	switch (fMaxUL->currentItem())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gWin->fSettings->SetMaxUploads(fMaxUL->currentItem() + 1); break;
		case 5:
			gWin->fSettings->SetMaxUploads(WSettings::Ten); break;
		case 6:
			gWin->fSettings->SetMaxUploads(WSettings::Fifteen); break;
		case 7:
			gWin->fSettings->SetMaxUploads(WSettings::Twenty); break;
		case 8:
			gWin->fSettings->SetMaxUploads(WSettings::Thirty); break;
		case 9:
			gWin->fSettings->SetMaxUploads(WSettings::Unlimited); break;
	}

	gWin->fSettings->SetFontSize(fFontSize->value());

#ifdef __linux__
	// save our launcher settings
	gWin->fSettings->SetMailLauncher(fMailtoLauncher->text());
	gWin->fSettings->SetHTTPLauncher(fHTTPLauncher->text());
	gWin->fSettings->SetFTPLauncher(fFTPLauncher->text());
	gWin->fSettings->SetDefaultLauncher(fDefaultLauncher->text());
#endif
	
	gWin->fSettings->SetULLimit(fULLimit->currentItem());
	gWin->fSettings->SetDLLimit(fDLLimit->currentItem());
	gWin->fSettings->SetBLLimit(fBLLimit->currentItem());
	gWin->fSettings->SetChatLimit(fChatLimit->currentItem());

	switch (fPacketSize->currentItem())
	{
		case 0:
			gWin->fSettings->SetPacketSize(1); break;
		case 1:
			gWin->fSettings->SetPacketSize(2); break;
		case 2:
			gWin->fSettings->SetPacketSize(4); break;
		case 3:
			gWin->fSettings->SetPacketSize(8); break;
		case 4:
			gWin->fSettings->SetPacketSize(16); break;
		case 5:
			gWin->fSettings->SetPacketSize(32); break;
	}

	gWin->fSettings->SetMinQueued( fMinQueued->currentItem() );
	
	gWin->fSettings->SetLogging(fLogging->isChecked());

	bool o;
	unsigned long bp;
	bp = fBasePort->text().toULong(&o);
	if (o)
	{
		gWin->fSettings->SetBasePort(bp);
	}
	else
	{
		gWin->fSettings->SetBasePort(7000);
	}
	// ok
	accept();
}

void
WPrefs::Cancel()
{
	reject();
}

void
WPrefs::StyleSelected(int id)
{
	// check style by index
	/*
	 * 0 - CDE
	 * 1 - Motif
	 * 2 - Motif Plus
	 * 3 - Platinum
	 * 4 - SGI
	 * 5 - Windows
	 */
	switch (id)
	{
		case 0:
			qApp->setStyle(new QCDEStyle);
			gWin->fSettings->SetStyle(WinShareWindow::CDE);
			break;
		case 1:
			qApp->setStyle(new QMotifStyle);
			gWin->fSettings->SetStyle(WinShareWindow::Motif);
			break;
		case 2:
			qApp->setStyle(new QMotifPlusStyle);
			gWin->fSettings->SetStyle(WinShareWindow::MotifPlus);
			break;
		case 3:
			qApp->setStyle(new QPlatinumStyle);
			gWin->fSettings->SetStyle(WinShareWindow::Platinum);
			break;
		case 4:
			qApp->setStyle(new QSGIStyle);
			gWin->fSettings->SetStyle(WinShareWindow::SGI);
			break;
		case 5:
			qApp->setStyle(new QWindowsStyle);
			gWin->fSettings->SetStyle(WinShareWindow::Windows);
			break;
		default:
			break;		// unknown style
	}
}

void
WPrefs::ColorSelected(int index)
{
	QString str;
	str = gWin->fSettings->GetColorItem(index);

	UpdateDescription(index);
}

void
WPrefs::UpdateDescription(int index)
{
	fCurColorIndex = index;

	fPreview->setText(tr("<font color=\"%1\">%2</font>").arg(fColor[index]).arg(tr(kSampleText[index])));
	fDesc->setText(tr(kColorDesc[index]));
}

void
WPrefs::ChangeColor()
{
	if (fCurColorIndex >= 0)
	{
		PRINT("ChangeColor()\n");
		QColor col(fColor[fCurColorIndex]);
		col = QColorDialog::getColor(col, this);
		fColor[fCurColorIndex] = col.name();
		UpdateDescription(fCurColorIndex);	// update the color (if changed)
	}
}

void
WPrefs::AwaySelected(int index)
{
	gWin->fSettings->SetAutoAway(index);
}

