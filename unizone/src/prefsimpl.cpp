#include "prefsimpl.h"
#include "prefs.h"
#include "global.h"
#include "settings.h"
#include "colors.h"
#include "debugimpl.h"
#include "lang.h"				// <postmaster@raasu.org> 20020924
#include "platform.h"			// <postmaster@raasu.org> 20021114

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

const char * kColorDesc[11] = {	MSG_HLOCAL_NAME, 
					MSG_HREMOTE_NAME,
					MSG_HREGULAR_TEXT,
					MSG_HSYSTEM_TEXT,
					MSG_HPING_TEXT,
					MSG_HERROR_TEXT,
					MSG_HERRORMSG_TEXT,
					MSG_HPRIVATE_TEXT,
					MSG_HACTION_TEXT,
					MSG_HURL_TEXT,
					MSG_HNAME_SAID_TEXT
								};

const char * kSampleText[11] = {	"Username",
									"Remote User",
									"Sample text",
									"<b>System</b>",
									"Sample text",
									"<b>Error</b>",
									"Error text",
									"Private text",
									"<b>Action</b>",
									"<u>http://magep.com/</u>",
									"Username"
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

	InitLanguage();

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
	fChat->setChecked(gWin->fSettings->GetChat());
	fPrivate->setChecked(gWin->fSettings->GetPrivate());
	fInfo->setChecked(gWin->fSettings->GetInfo());
	fWarning->setChecked(gWin->fSettings->GetWarning());
	fError->setChecked(gWin->fSettings->GetError());
	
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
#endif

	fChatLimit->setCurrentItem(gWin->fSettings->GetChatLimit());
	fULLimit->setCurrentItem(gWin->fSettings->GetULLimit());
	fDLLimit->setCurrentItem(gWin->fSettings->GetDLLimit());
	fBLLimit->setCurrentItem(gWin->fSettings->GetBLLimit());

	fLogging->setChecked(gWin->fSettings->GetLogging());
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
	gWin->fSettings->SetMultiColor(fMultiColor->isChecked());
	gWin->fSettings->SetTimeStamps(fTimeStamps->isChecked());
	gWin->fSettings->SetUserEvents(fUserEvents->isChecked());
	gWin->fSettings->SetUploads(fUploads->isChecked());
	gWin->fSettings->SetChat(fChat->isChecked());
	gWin->fSettings->SetPrivate(fPrivate->isChecked());
	gWin->fSettings->SetInfo(fInfo->isChecked());
	gWin->fSettings->SetWarning(fWarning->isChecked());
	gWin->fSettings->SetError(fError->isChecked());

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
#endif
	
	gWin->fSettings->SetULLimit(fULLimit->currentItem());
	gWin->fSettings->SetDLLimit(fDLLimit->currentItem());
	gWin->fSettings->SetBLLimit(fBLLimit->currentItem());
	gWin->fSettings->SetChatLimit(fChatLimit->currentItem());
	
	gWin->fSettings->SetLogging(fLogging->isChecked());
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

void
WPrefs::InitLanguage()
{
	// Initialize dialog for Languages
	this->setCaption(tr(MSG_PR_PREFERENCES));

	// Tabs
	fTabs->changeTab(fGeneral, tr(MSG_GENERAL));
	fTabs->changeTab(fConnection, tr(MSG_CONNECTION));
	fTabs->changeTab(fDisplay, tr(MSG_DISPLAY));
	fTabs->changeTab(fColors, tr(MSG_COLORS));
	fTabs->changeTab(fStyle, tr(MSG_STYLE));
	fTabs->changeTab(fFileSharing, tr(MSG_FILE_SHARING));
	fTabs->changeTab(fURLLaunching, tr(MSG_URL_LAUNCHING));
	fTabs->changeTab(fThrottling, tr(MSG_THROTTLING));

	// Auto away
	TextLabel1->setText(tr(MSG_CAUTOAWAY));
	fAutoAway->clear();
	fAutoAway->insertItem(tr(MSG_DISABLED));
	fAutoAway->insertItem(tr(MSG_2_MINUTES));
	fAutoAway->insertItem(tr(MSG_5_MINUTES));
	fAutoAway->insertItem(tr(MSG_10_MINUTES));
	fAutoAway->insertItem(tr(MSG_15_MINUTES));
	fAutoAway->insertItem(tr(MSG_20_MINUTES));
	fAutoAway->insertItem(tr(MSG_30_MINUTES));
	fAutoAway->insertItem(tr(MSG_1_HOUR));
	fAutoAway->insertItem(tr(MSG_2_HOURS));
	
	// Misc general
	fAutoUpdateServers->setProperty("text", tr(MSG_AUTOUPDATE));
	fNewVersions->setProperty("text", tr(MSG_CHECK_NEW));
	fLoginStartup->setProperty("text", tr(MSG_LOGIN_ON_START));
	fLogging->setProperty("text", tr(MSG_ENABLE_LOGGING));
	fMultiColor->setProperty("text", tr(MSG_MULTI_COLOR_LISTVIEWS));

	// Connection
	TextLabel2->setText(tr(MSG_CUPLOAD_BAND));
	fFireWalled->setProperty("text", tr(MSG_FIREWALLED));

	// Display
	fWarning->setProperty("text", tr(MSG_WARNING_MSGS));
	fError->setProperty("text", tr(MSG_ERROR_MSGS));
	fPrivate->setProperty("text", tr(MSG_PRIVATE_MSGS));
	fUserEvents->setProperty("text", tr(MSG_USEREVENTS));
	fChat->setProperty("text", tr(MSG_CHAT));
	fInfo->setProperty("text", tr(MSG_INFO_MSGS));
	fUploads->setProperty("text", tr(MSG_UPLOADS));
	fTimeStamps->setProperty("text", tr(MSG_TIMESTAMPS));
	fFlashMain->setProperty("text", tr(MSG_FLASH_WINDOW));
	fFlashPrivate->setProperty("text", tr(MSG_FLASH_PRIVATE));
	GroupBox1->setProperty("title", tr(MSG_FONT_SIZE));

	// Colors
	fColorsList->clear();
	fColorsList->insertItem(tr(MSG_LOCAL_NAME));
	fColorsList->insertItem(tr(MSG_REMOTE_NAME));
	fColorsList->insertItem(tr(MSG_REGULAR_TEXT));
	fColorsList->insertItem(tr(MSG_SYSTEM_TEXT));
	fColorsList->insertItem(tr(MSG_PING_TEXT));
	fColorsList->insertItem(tr(MSG_ERROR_TEXT));
	fColorsList->insertItem(tr(MSG_ERRORMSG_TEXT));
	fColorsList->insertItem(tr(MSG_PRIVATE_TEXT));
	fColorsList->insertItem(tr(MSG_ACTION_TEXT));
	fColorsList->insertItem(tr(MSG_URL_TEXT));
	fColorsList->insertItem(tr(MSG_NAME_SAID_TEXT));
	TextLabel5->setProperty("text", tr(MSG_CDESCRIPTION));
	TextLabel3->setProperty("text", tr(MSG_CPREVIEW));
	fChange->setProperty("text", tr(MSG_CHANGE));	

	// Style
	TextLabel7->setProperty("text", tr(MSG_HSTYLE));

	// File sharing
	fFileSharingEnabled->setProperty("text", tr(MSG_FS_ENABLED));
	fBinkyNuke->setProperty("text", tr(MSG_BINKYNUKE));
	fBlockDisconnected->setProperty("text", tr(MSG_BLOCK_DISCONNECTED));
	TextLabel1_2_3->setProperty("text", tr(MSG_CFS_MAXUP));
	TextLabel1_2_3_3->setProperty("text", tr(MSG_CFS_MAXDOWN));
	fMaxUL->removeItem(9);	// remove "Unlimited"
	fMaxUL->insertItem(tr(MSG_UNLIMITED));
	fMaxDL->removeItem(9);
	fMaxDL->insertItem(tr(MSG_UNLIMITED));

	// launchers
	TextLabel1_2->setProperty("text", tr(MSG_CMAILTO_LAUNCHER));
	TextLabel1_2_2->setProperty("text", tr(MSG_CHTTP_LAUNCHER));
	TextLabel1_2_2_2->setProperty("text", tr(MSG_CFTP_LAUNCHER));

	// throttling
	TextLabel1_2_4->setProperty("text", tr(MSG_CCHAT));
	TextLabel1_2_4_2->setProperty("text", tr(MSG_TH_UPLOADS));
	TextLabel1_2_4_3->setProperty("text", tr(MSG_TH_DOWNLOADS));
	TextLabel1_2_4_4->setProperty("text", tr(MSG_TH_BLOCKED));
	fChatLimit->removeItem(0);
	fChatLimit->insertItem(tr(MSG_NO_LIMIT), 0);
	fULLimit->removeItem(0);
	fULLimit->insertItem(tr(MSG_NO_LIMIT), 0);
	fDLLimit->removeItem(0);
	fDLLimit->insertItem(tr(MSG_NO_LIMIT), 0);
	fBLLimit->removeItem(0);
	fBLLimit->insertItem(tr(MSG_NO_LIMIT), 0);

	fOK->setProperty("text", tr(MSG_OK));
	fCancel->setProperty("text", tr(MSG_CANCEL));
}
