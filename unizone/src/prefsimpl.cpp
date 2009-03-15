#include <qapplication.h>
#if !defined(QT_NO_STYLE_MOTIF)
#include <qmotifstyle.h>
#endif
#if !defined(QT_NO_STYLE_WINDOWS)
#include <qwindowsstyle.h>
#endif
#if !defined(QT_NO_STYLE_WINDOWSXP)
#include <qwindowsxpstyle.h>
#endif
#if !defined(QT_NO_STYLE_CDE)
#include <qcdestyle.h>
#endif
#if defined(__APPLE__)
# if !defined(QT_NO_STYLE_MAC)
#  include <qmacstyle_mac.h>
# endif
#endif
#if !defined(QT_NO_STYLE_PLASTIQUE)
#include <qplastiquestyle.h>
#endif
#if !defined(QT_NO_STYLE_WINDOWSVISTA)
#include <qwindowsvistastyle.h>
#endif
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <q3listbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <q3filedialog.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qlineedit.h>

#include "prefsimpl.h"
#include "ui_prefs.h"
#include "global.h"
#include "settings.h"
#include "colors.h"
#include "debugimpl.h"


const char * kColorDesc[13] = {
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
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of your user name in text when someone says your name in the main chat." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of \"Warning\"." ),
					QT_TRANSLATE_NOOP( "WPrefs", "This is the color of the text in warning messages." )
								};

const char * kSampleText[13] = {
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
									QT_TRANSLATE_NOOP( "WPrefs", "Username" ),
									QT_TRANSLATE_NOOP( "WPrefs", "<b>Warning</b>" ),
									QT_TRANSLATE_NOOP( "WPrefs", "Warning text" )
								};

/*
 *  Constructs a prefs which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WPrefs::WPrefs( QWidget* parent,  const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent, name, modal, fl )
{
	ui = new Ui_WPrefsBase();
	ui->setupUi(this);

	if ( !name )
		setName( "WPrefs" );

	connect(ui->fOK, SIGNAL(clicked()), this, SLOT(OK()));
	connect(ui->fCancel, SIGNAL(clicked()), this, SLOT(Cancel()));
	connect(ui->fStyleList, SIGNAL(highlighted(int)), this, SLOT(StyleSelected(int)));
	connect(ui->fColorsList, SIGNAL(highlighted(int)), this, SLOT(ColorSelected(int)));
	connect(ui->fChange, SIGNAL(clicked()), this, SLOT(ChangeColor()));
	connect(ui->fSound, SIGNAL(clicked()), this, SLOT(ChangeSound()));
	connect(ui->fResetSound, SIGNAL(clicked()), this, SLOT(ResetSound()));
	connect(ui->fAutoAway, SIGNAL(highlighted(int)), this, SLOT(AwaySelected(int)));

	fCurColorIndex = -1;

	// initialize the colors
	for (int i = 0; i < WColors::NumColors; i++)
	{
		fColor[i] = gWin->fSettings->GetColorItem(i);
	}

	ui->fColorsList->insertItem(tr("Local Name"));
	ui->fColorsList->insertItem(tr("Remote Name"));
	ui->fColorsList->insertItem(tr("Regular Text"));
	ui->fColorsList->insertItem(tr("System Text"));
	ui->fColorsList->insertItem(tr("Ping Text"));
	ui->fColorsList->insertItem(tr("Error Text"));
	ui->fColorsList->insertItem(tr("Error Message Text"));
	ui->fColorsList->insertItem(tr("Private Text"));
	ui->fColorsList->insertItem(tr("Action Text"));
	ui->fColorsList->insertItem(tr("URL Text"));
	ui->fColorsList->insertItem(tr("'Name Said' Text"));
	ui->fColorsList->insertItem(tr("Warning Text"));
	ui->fColorsList->insertItem(tr("Warning Message Text"));

	ui->fAutoUpdateServers->setChecked(gWin->fSettings->GetAutoUpdateServers());
	ui->fNewVersions->setChecked(gWin->fSettings->GetCheckNewVersions());
	ui->fLoginStartup->setChecked(gWin->fSettings->GetLoginOnStartup());
	ui->fFireWalled->setChecked(gWin->fSettings->GetFirewalled());
	ui->fBinkyNuke->setChecked(gWin->fSettings->GetBinkyNuke());
	ui->fBlockDisconnected->setChecked(gWin->fSettings->GetBlockDisconnected());
	ui->fPreservePaths->setChecked(gWin->fSettings->GetPreservePaths());
	ui->fAutoClear->setChecked(gWin->fSettings->GetAutoClear());
	ui->fAutoClose->setChecked(gWin->fSettings->GetAutoClose());
	ui->fMultiColor->setChecked(gWin->fSettings->GetMultiColor());
	if (gWin->fSettings->GetConnection() != qApp->translate("Connection", "Unknown"))
	{
		for (int i = 0; i < ui->fBandwidth->count(); i++)
		{
			if (ui->fBandwidth->text(i) == gWin->fSettings->GetConnection())
			{
				ui->fBandwidth->setCurrentItem(i);
				break;
			}
		}
	}

	ui->fHTTPproxy->setText(gWin->fSettings->GetHTTPProxy());
	ui->fHTTPport->setText(QString::number(gWin->fSettings->GetHTTPPort()));

	ui->fTimeStamps->setChecked(gWin->fSettings->GetTimeStamps());
	ui->fUserEvents->setChecked(gWin->fSettings->GetUserEvents());
	ui->fUploads->setChecked(gWin->fSettings->GetUploads());
	ui->fDownloads->setChecked(gWin->fSettings->GetDownloads());
	ui->fChat->setChecked(gWin->fSettings->GetChat());
	ui->fPrivate->setChecked(gWin->fSettings->GetPrivate());
	ui->fInfo->setChecked(gWin->fSettings->GetInfo());
	ui->fWarning->setChecked(gWin->fSettings->GetWarning());
	ui->fError->setChecked(gWin->fSettings->GetError());
	ui->fSounds->setChecked(gWin->fSettings->GetSounds());
	ui->fSoundFile->setText(gWin->fSettings->GetSoundFile());
	ui->fIPAddresses->setChecked(gWin->fSettings->GetIPAddresses());

	ui->fStyleList->insertItem(tr("CDE"));
#if defined(QT_NO_STYLE_CDE)
	ui->fStyleList->item(0)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("Motif"));
#if defined(QT_NO_STYLE_MOTIF)
	ui->fStyleList->item(1)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("WindowsXP"));
#if !defined(_WIN32) || defined(QT_NO_STYLE_WINDOWSXP)
	ui->fStyleList->item(2)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("Windows"));
#if defined(QT_NO_STYLE_WINDOWS)
	ui->fStyleList->item(3)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("Mac"));
#if !defined(__APPLE__) || defined(QT_NO_STYLE_MAC)
	ui->fStyleList->item(4)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("Plastique"));
#if defined(QT_NO_STYLE_PLASTIQUE)
	ui->fStyleList->item(5)->setSelectable(false);
#endif
	ui->fStyleList->insertItem(tr("WindowsVista"));
#if !defined(_WIN32) || defined(QT_NO_STYLE_WINDOWSVISTA)
	ui->fStyleList->item(6)->setSelectable(false);
#endif

	switch (gWin->fSettings->GetStyle())
	{
		case WinShareWindow::CDE:
			ui->fStyleList->setCurrentItem(0);
			break;

		case WinShareWindow::Motif:
			ui->fStyleList->setCurrentItem(1);
			break;

		case WinShareWindow::WindowsXP:
			ui->fStyleList->setCurrentItem(2);
			break;

		case WinShareWindow::Windows:
			ui->fStyleList->setCurrentItem(3);
			break;

		case WinShareWindow::Mac:
			ui->fStyleList->setCurrentItem(4);
			break;

		case WinShareWindow::Plastique:
			ui->fStyleList->setCurrentItem(5);
			break;

		case WinShareWindow::WindowsVista:
			ui->fStyleList->setCurrentItem(6);
			break;
	}

	// init auto away
	ui->fAutoAway->setCurrentItem(gWin->fSettings->GetAutoAway());

#ifdef _WIN32
	// init flash flags
	if (gWin->fSettings->GetFlash() & WSettings::FlashMain)
		ui->fFlashMain->setChecked(true);
	else
		ui->fFlashMain->setChecked(false);

	if (gWin->fSettings->GetFlash() & WSettings::FlashPriv)
		ui->fFlashPrivate->setChecked(true);
	else
		ui->fFlashPrivate->setChecked(false);
#else
	// Linux, FreeBSD, QNX and SunOS/Solaris don't get this nifty feature
	ui->fFlashMain->hide();
	ui->fFlashPrivate->hide();
#endif
	ui->fEmptyWindows->setCurrentItem(gWin->fSettings->GetEmptyWindows());

	switch (gWin->fSettings->GetMaxDownloads())
	{
		case WSettings::One:
			ui->fMaxDL->setCurrentItem(0); break;
		case WSettings::Two:
			ui->fMaxDL->setCurrentItem(1); break;
		case WSettings::Three:
			ui->fMaxDL->setCurrentItem(2); break;
		case WSettings::Four:
			ui->fMaxDL->setCurrentItem(3); break;
		case WSettings::Five:
			ui->fMaxDL->setCurrentItem(4); break;
		case WSettings::Ten:
			ui->fMaxDL->setCurrentItem(5); break;
		case WSettings::Fifteen:
			ui->fMaxDL->setCurrentItem(6); break;
		case WSettings::Twenty:
			ui->fMaxDL->setCurrentItem(7); break;
		case WSettings::Thirty:
			ui->fMaxDL->setCurrentItem(8); break;
		case WSettings::Unlimited:
			ui->fMaxDL->setCurrentItem(9); break;
	}

	switch (gWin->fSettings->GetMaxUploads())
	{
		case WSettings::One:
			ui->fMaxUL->setCurrentItem(0); break;
		case WSettings::Two:
			ui->fMaxUL->setCurrentItem(1); break;
		case WSettings::Three:
			ui->fMaxUL->setCurrentItem(2); break;
		case WSettings::Four:
			ui->fMaxUL->setCurrentItem(3); break;
		case WSettings::Five:
			ui->fMaxUL->setCurrentItem(4); break;
		case WSettings::Ten:
			ui->fMaxUL->setCurrentItem(5); break;
		case WSettings::Fifteen:
			ui->fMaxUL->setCurrentItem(6); break;
		case WSettings::Twenty:
			ui->fMaxUL->setCurrentItem(7); break;
		case WSettings::Thirty:
			ui->fMaxUL->setCurrentItem(8); break;
		case WSettings::Unlimited:
			ui->fMaxUL->setCurrentItem(9); break;
	}

	ui->fFileSharingEnabled->setChecked(gWin->fSettings->GetSharingEnabled());
	PRINT("Setting font size\n");
	ui->fFontSize->setValue(gWin->fSettings->GetFontSize());

#ifdef _WIN32	// windows has a system based launcher
	ui->fTabs->setTabEnabled(ui->fURLLaunching, false);
#else
	ui->fMailtoLauncher->setText(gWin->fSettings->GetMailLauncher());
	ui->fHTTPLauncher->setText(gWin->fSettings->GetHTTPLauncher());
	ui->fFTPLauncher->setText(gWin->fSettings->GetFTPLauncher());
	ui->fDefaultLauncher->setText(gWin->fSettings->GetDefaultLauncher());
#endif
#if defined(QT_NO_STYLE) || defined(DISABLE_STYLES)
	ui->fTabs->setTabEnabled(fStyle, false);
#endif

	ui->fChatLimit->setCurrentItem(gWin->fSettings->GetChatLimit());
	ui->fULLimit->setCurrentItem(gWin->fSettings->GetULLimit());
	ui->fDLLimit->setCurrentItem(gWin->fSettings->GetDLLimit());
	ui->fBLLimit->setCurrentItem(gWin->fSettings->GetBLLimit());

	double ps = gWin->fSettings->GetPacketSize();
	if (ps == 0.5)
		ui->fPacketSize->setCurrentItem(0);
	else if (ps == 1)
		ui->fPacketSize->setCurrentItem(1);
	else if (ps == 2)
		ui->fPacketSize->setCurrentItem(2);
	else if (ps == 4)
		ui->fPacketSize->setCurrentItem(3);
	else if (ps == 8)
		ui->fPacketSize->setCurrentItem(4);
	else if (ps == 16)
		ui->fPacketSize->setCurrentItem(5);
	else if (ps == 32)
		ui->fPacketSize->setCurrentItem(6);
	else if (ps == 64)
		ui->fPacketSize->setCurrentItem(7);
	else if (ps == 128)
		ui->fPacketSize->setCurrentItem(8);
	else if (ps == 256)
		ui->fPacketSize->setCurrentItem(9);
	else if (ps == 512)
		ui->fPacketSize->setCurrentItem(10);
	else if (ps == 1024)
		ui->fPacketSize->setCurrentItem(11);

	ui->fMinQueued->setCurrentItem( gWin->fSettings->GetMinQueued() );

	ui->fLogging->setChecked(gWin->fSettings->GetLogging());

	ui->fBasePort->setText( QString::number( gWin->fSettings->GetBasePort() ) );

	ui->fPortRange->setText( QString::number( gWin->fSettings->GetPortRange() ) );
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
	for (i = 0; i < WColors::NumColors; i++)
		gWin->fSettings->AddColorItem(fColor[i]);

	// save all the other stuff
	gWin->fSettings->SetAutoUpdateServers(ui->fAutoUpdateServers->isChecked());
	gWin->fSettings->SetCheckNewVersions(ui->fNewVersions->isChecked());
	gWin->fSettings->SetLoginOnStartup(ui->fLoginStartup->isChecked());
	gWin->fSettings->SetConnection(ui->fBandwidth->currentText());
	gWin->fSettings->SetHTTPProxy(ui->fHTTPproxy->text());
	gWin->fSettings->SetHTTPPort(ui->fHTTPport->text().toUInt());
	gWin->fSettings->SetFirewalled(ui->fFireWalled->isChecked());
	gWin->fSettings->SetBinkyNuke(ui->fBinkyNuke->isChecked());
	gWin->fSettings->SetBlockDisconnected(ui->fBlockDisconnected->isChecked());
	gWin->fSettings->SetPreservePaths(ui->fPreservePaths->isChecked());
	gWin->fSettings->SetAutoClear(ui->fAutoClear->isChecked());
	gWin->fSettings->SetAutoClose(ui->fAutoClose->isChecked());
	gWin->fSettings->SetMultiColor(ui->fMultiColor->isChecked());
	gWin->fSettings->SetTimeStamps(ui->fTimeStamps->isChecked());
	gWin->fSettings->SetUserEvents(ui->fUserEvents->isChecked());
	gWin->fSettings->SetUploads(ui->fUploads->isChecked());
	gWin->fSettings->SetDownloads(ui->fDownloads->isChecked());
	gWin->fSettings->SetChat(ui->fChat->isChecked());
	gWin->fSettings->SetPrivate(ui->fPrivate->isChecked());
	gWin->fSettings->SetInfo(ui->fInfo->isChecked());
	gWin->fSettings->SetWarning(ui->fWarning->isChecked());
	gWin->fSettings->SetError(ui->fError->isChecked());
	gWin->fSettings->SetSounds(ui->fSounds->isChecked());
	gWin->fSettings->SetSoundFile(ui->fSoundFile->text());
	gWin->fSettings->SetIPAddresses(ui->fIPAddresses->isChecked());

	// flash settings
	int flags = WSettings::FlashNone;
	if (ui->fFlashMain->isChecked())
		flags |= WSettings::FlashMain;
	if (ui->fFlashPrivate->isChecked())
		flags |= WSettings::FlashPriv;
	gWin->fSettings->SetFlash(flags);
	gWin->fSettings->SetEmptyWindows(ui->fEmptyWindows->currentItem());

	gWin->fSettings->SetSharingEnabled(ui->fFileSharingEnabled->isChecked());

	switch (ui->fMaxDL->currentItem())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gWin->fSettings->SetMaxDownloads(ui->fMaxDL->currentItem() + 1); break;
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

	switch (ui->fMaxUL->currentItem())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gWin->fSettings->SetMaxUploads(ui->fMaxUL->currentItem() + 1); break;
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

	gWin->fSettings->SetFontSize(ui->fFontSize->value());

#ifndef WIN32
	// save our launcher settings
	gWin->fSettings->SetMailLauncher(ui->fMailtoLauncher->text());
	gWin->fSettings->SetHTTPLauncher(ui->fHTTPLauncher->text());
	gWin->fSettings->SetFTPLauncher(ui->fFTPLauncher->text());
	gWin->fSettings->SetDefaultLauncher(ui->fDefaultLauncher->text());
#endif

	gWin->fSettings->SetULLimit(ui->fULLimit->currentItem());
	gWin->fSettings->SetDLLimit(ui->fDLLimit->currentItem());
	gWin->fSettings->SetBLLimit(ui->fBLLimit->currentItem());
	gWin->fSettings->SetChatLimit(ui->fChatLimit->currentItem());

	switch (ui->fPacketSize->currentItem())
	{
		case 0:
			gWin->fSettings->SetPacketSize(0.5); break;
		case 1:
			gWin->fSettings->SetPacketSize(1); break;
		case 2:
			gWin->fSettings->SetPacketSize(2); break;
		case 3:
			gWin->fSettings->SetPacketSize(4); break;
		case 4:
			gWin->fSettings->SetPacketSize(8); break;
		case 5:
			gWin->fSettings->SetPacketSize(16); break;
		case 6:
			gWin->fSettings->SetPacketSize(32); break;
		case 7:
			gWin->fSettings->SetPacketSize(64); break;
		case 8:
			gWin->fSettings->SetPacketSize(128); break;
		case 9:
			gWin->fSettings->SetPacketSize(256); break;
		case 10:
			gWin->fSettings->SetPacketSize(512); break;
		case 11:
			gWin->fSettings->SetPacketSize(1024); break;
	}

	gWin->fSettings->SetMinQueued(ui->fMinQueued->currentItem() );

	gWin->fSettings->SetLogging(ui->fLogging->isChecked());

	bool o;
	unsigned long bp,pr;

	bp = ui->fBasePort->text().toULong(&o);
	gWin->fSettings->SetBasePort(o ? bp : 7000);

	pr = ui->fPortRange->text().toULong(&o);
	gWin->fSettings->SetPortRange(o ? pr : 100);

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
	 * 3 - WindowsXP
	 * 4 - Windows
	 * 5 - Aqua
	 * 6 - Plastique
	 * 7 - WindowsVista
	 */
#ifndef DISABLE_STYLES
	switch (id)
	{
		case 0:
#if !defined(QT_NO_STYLE_CDE)
			qApp->setStyle(new QCDEStyle);
#endif
			gWin->fSettings->SetStyle(WinShareWindow::CDE);
			break;
		case 1:
#if !defined(QT_NO_STYLE_MOTIF)
			qApp->setStyle(new QMotifStyle);
#endif
			gWin->fSettings->SetStyle(WinShareWindow::Motif);
			break;
		case 2:
#if defined(_WIN32)
# if !defined(QT_NO_STYLE_WINDOWSXP)
			qApp->setStyle(new QWindowsXPStyle);
# endif
#endif
			gWin->fSettings->SetStyle(WinShareWindow::WindowsXP);
			break;
		case 3:
#if !defined(QT_NO_STYLE_WINDOWS)
			qApp->setStyle(new QWindowsStyle);
#endif
			gWin->fSettings->SetStyle(WinShareWindow::Windows);
			break;
		case 4:
#if defined(__APPLE__)
# if !defined(QT_NO_STYLE_MAC)
			qApp->setStyle(new QMacStyle);
# endif
#endif
			gWin->fSettings->SetStyle(WinShareWindow::Mac);
			break;
		case 5:
#if !defined(QT_NO_STYLE_PLASTIQUE)
			qApp->setStyle(new QPlastiqueStyle);
#endif
			gWin->fSettings->SetStyle(WinShareWindow::Plastique);
			break;
		case 6:
#if defined(_WIN32)
# if !defined(QT_NO_STYLE_WINDOWSVISTA)
			qApp->setStyle(new QWindowsVistaStyle);
# endif
#endif
			gWin->fSettings->SetStyle(WinShareWindow::WindowsVista);
			break;
		default:
			break;		// unknown style
	}
#endif
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

	ui->fPreview->setText(tr("<font color=\"%1\">%2</font>").arg(fColor[index]).arg(tr(kSampleText[index])));
	ui->fDesc->setText(tr(kColorDesc[index]));
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
WPrefs::ChangeSound()
{
	QString fn = Q3FileDialog::getOpenFileName(QString::null, "*.wav", this);
	if (!fn.isEmpty())
		ui->fSoundFile->setText(fn);
}

void
WPrefs::ResetSound()
{
	ui->fSoundFile->clear();
}


void
WPrefs::AwaySelected(int index)
{
	gWin->fSettings->SetAutoAway(index);
}

