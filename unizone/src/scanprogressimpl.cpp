#include <qlabel.h>

#include "scanprogressimpl.h"
#include "scanprogress.h"

ScanProgress::ScanProgress(QObject * owner, QWidget* parent, const char* name, bool modal, WFlags fl)
: ScanProgressBase(parent, name, modal, fl)
{
	if (!name)
		setName( "ScanProgress" );
}

ScanProgress::~ScanProgress()
{
}

void
ScanProgress::SetDirsLeft(int dl)
{
	fDirsLeft->setText(QString::number(dl));
}

void
ScanProgress::SetScanDirectory(QString dir)
{
	fDirectory->setText(dir);
}

void
ScanProgress::SetScanFile(QString file)
{
	fFile->setText(file);
}

void
ScanProgress::SetScannedDirs(int sd)
{
	fDirsScanned->setText(QString::number(sd));
}

void
ScanProgress::SetScannedFiles(int sf)
{
	fFilesScanned->setText(QString::number(sf));
}