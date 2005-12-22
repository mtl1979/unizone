#include <qlabel.h>
#include <qdir.h>

#include "scanprogressimpl.h"
#include "scanprogress.h"
#include "scanevent.h"

ScanProgress::ScanProgress(QWidget* parent, const char* name, bool modal, WFlags fl)
: ScanProgressBase(parent, name, modal, fl)
{
	if (!name)
		setName( "ScanProgress" );
}

ScanProgress::~ScanProgress()
{
	// empty
}

void
ScanProgress::SetDirsLeft(int dl)
{
	QString sdl;
	sdl.setNum(dl);
	fDirsLeft->setText(sdl);	
}

void
ScanProgress::SetScanDirectory(const QString & dir)
{
	
	fDirectory->setText(QDir::convertSeparators(dir));
}

void
ScanProgress::SetScanFile(const QString & file)
{
	fFile->setText(file);	
}

void
ScanProgress::SetScannedDirs(int sd)
{
	QString ssd;
	ssd.setNum(sd); 
	fDirsScanned->setText(ssd);
}

void
ScanProgress::SetScannedFiles(int sf)
{
	QString ssf;
	ssf.setNum(sf);
	fFilesScanned->setText(ssf);
}

void
ScanProgress::reset()
{
	SetDirsLeft(0);
	SetScanDirectory("-");
	SetScanFile("-");
	SetScannedDirs(0);
	SetScannedFiles(0);
}

void
ScanProgress::customEvent(QCustomEvent *e)
{
	ScanEvent * se = dynamic_cast<ScanEvent *>(e);
	if (se)
	{
		switch ((int) se->type())
		{
		case SET::ScanDirectory:
			{
				SetScanDirectory(se->text());
				break;
			}
		case SET::ScanFile:
			{
				SetScanFile(se->text());
				break;
			}
		case SET::ScannedDirs:
			{
				SetScannedDirs(se->number());
				break;
			}
		case SET::ScannedFiles:
			{
				SetScannedFiles(se->number());
				break;
			}
		case SET::DirsLeft:
			{
				SetDirsLeft(se->number());
				break;
			}
		case SET::Reset:
			{
				reset();
				break;
			}
		case SET::Show:
			{
				show();
				break;
			}
		case SET::Hide:
			{
				hide();
				break;
			}
		}
	}
}
