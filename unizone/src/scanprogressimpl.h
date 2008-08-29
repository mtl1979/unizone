#ifndef SCANPROGRESS_H
#define SCANPROGRESS_H

#include "scanprogress.h"

#if defined(WIN32) && (_MSC_VER < 1400)
# define SET ScanEvent::Type
#else
# define SET ScanEvent
#endif



class ScanProgress : public ScanProgressBase
{
public:

    ScanProgress(QWidget* parent = 0, 	const char* name = 0, bool modal = false, 
				WFlags fl = WStyle_Customize | WStyle_NormalBorder | WStyle_Title | 
							WStyle_Minimize | WStyle_Maximize);
	~ScanProgress();

protected:
	void customEvent(QCustomEvent *);
private:
	void SetScanDirectory(const QString &dir);
	void SetScanFile(const QString &file);
	void SetScannedDirs(int sd);
	void SetScannedFiles(int sf);
	void SetDirsLeft(int dl);
	void reset();
};
#endif
