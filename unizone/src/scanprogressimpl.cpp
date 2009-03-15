#include <qlabel.h>
#include <qdir.h>
#include <QCustomEvent>

#include "scanprogressimpl.h"
#include "ui_scanprogress.h"
#include "scanevent.h"
#include "debugimpl.h"

ScanProgress::ScanProgress(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
: QDialog(parent, name, modal, fl)
{
	ui = new Ui_ScanProgressBase();
	ui->setupUi(this);

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
	ui->fDirsLeft->setText(sdl);	
}

void
ScanProgress::SetScanDirectory(const QString & dir)
{
	
	ui->fDirectory->setText(QDir::convertSeparators(dir));
}

void
ScanProgress::SetScanFile(const QString & file)
{
	ui->fFile->setText(file);	
}

void
ScanProgress::SetScannedDirs(int sd)
{
	QString ssd;
	ssd.setNum(sd); 
	ui->fDirsScanned->setText(ssd);
}

void
ScanProgress::SetScannedFiles(int sf)
{
	QString ssf;
	ssf.setNum(sf);
	ui->fFilesScanned->setText(ssf);
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
ScanProgress::customEvent(QEvent *e)
{
	PRINT("\tScanProgress::customEvent %i\n", e->type());
	ScanEvent * se = dynamic_cast<ScanEvent *>(e);
	if (se)
	{
		switch ((int) se->type())
		{
		case ScanEvent::ScanDirectory:
			{
				SetScanDirectory(se->text());
				break;
			}
		case ScanEvent::ScanFile:
			{
				SetScanFile(se->text());
				break;
			}
		case ScanEvent::ScannedDirs:
			{
				SetScannedDirs(se->number());
				break;
			}
		case ScanEvent::ScannedFiles:
			{
				SetScannedFiles(se->number());
				break;
			}
		case ScanEvent::DirsLeft:
			{
				SetDirsLeft(se->number());
				break;
			}
		case ScanEvent::Reset:
			{
				reset();
				break;
			}
		case ScanEvent::Show:
			{
				show();
				break;
			}
		case ScanEvent::Hide:
			{
				hide();
				break;
			}
		}
	}
}
