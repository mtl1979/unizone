#ifndef SCANPROGRESS_H
#define SCANPROGRESS_H

#include "scanprogress.h"

class ScanProgress : public ScanProgressBase
{
public:
    ScanProgress(QObject * owner = 0, QWidget* parent = 0, 	const char* name = 0, bool modal = false, 
				WFlags fl = WStyle_Customize | WStyle_NormalBorder | WStyle_Title | 
							WStyle_Minimize | WStyle_Maximize);
	~ScanProgress();

	void SetScanDirectory(QString dir);
	void SetScanFile(QString file);
	void SetScannedDirs(int sd);
	void SetScannedFiles(int sf);
	void SetDirsLeft(int dl);
};
#endif
