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
	fLock.lock();
	fDirsLeft->setText(QString::number(dl));
	fLock.unlock();
}

void
ScanProgress::SetScanDirectory(const QString & dir)
{
	fLock.lock();
	fDirectory->setText(dir);
	fLock.unlock();
}

void
ScanProgress::SetScanFile(const QString & file)
{
	fLock.lock();
	fFile->setText(file);
	fLock.unlock();
}

void
ScanProgress::SetScannedDirs(int sd)
{
	fLock.lock();
	fDirsScanned->setText(QString::number(sd));
	fLock.unlock();
}

void
ScanProgress::SetScannedFiles(int sf)
{
	fLock.lock();
	fFilesScanned->setText(QString::number(sf));
	fLock.unlock();
}
