#include <qlabel.h>

#include "scanprogressimpl.h"
#include "scanprogress.h"
#include "scanevent.h"

ScanProgress::ScanProgress(QObject * owner, QWidget* parent, const char* name, bool modal, WFlags fl)
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
	
	fDirectory->setText(dir);
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
#ifdef WIN32
		case ScanEvent::Type::ScanDirectory:
#else
		case ScanEvent::ScanDirectory:
#endif
			{
				SetScanDirectory(se->text());
				break;
			}
#ifdef WIN32
		case ScanEvent::Type::ScanFile:
#else
		case ScanEvent::ScanFile:
#endif
			{
				SetScanFile(se->text());
				break;
			}
#ifdef WIN32
		case ScanEvent::Type::ScannedDirs:
#else
		case ScanEvent::ScannedDirs:
#endif
			{
				SetScannedDirs(se->number());
				break;
			}
#ifdef WIN32
		case ScanEvent::Type::ScannedFiles:
#else
		case ScanEvent::ScannedFiles:
#endif
			{
				SetScannedFiles(se->number());
				break;
			}
#ifdef WIN32
		case ScanEvent::Type::DirsLeft:
#else
		case ScanEvent::DirsLeft:
#endif
			{
				SetDirsLeft(se->number());
				break;
			}
#ifdef WIN32
		case ScanEvent::Type::Reset:
#else
		case ScanEvent::Reset:
#endif
			{
				reset();
				break;
			}
		}
	}
}
