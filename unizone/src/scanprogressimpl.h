#ifndef SCANPROGRESSIMPL_H
#define SCANPROGRESSIMPL_H

#include "scanprogress.h"


class ScanProgress : public QDialog
{
public:

    ScanProgress(QWidget* parent = 0, 	const char* name = 0, bool modal = false, 
		Qt::WindowFlags fl = Qt::WStyle_Customize | Qt::WStyle_NormalBorder |
		Qt::WStyle_Title | Qt::WStyle_Minimize | Qt::WStyle_Maximize);
	~ScanProgress();

protected:
	void customEvent(QEvent *);

private:
	void SetScanDirectory(const QString &dir);
	void SetScanFile(const QString &file);
	void SetScannedDirs(int sd);
	void SetScannedFiles(int sf);
	void SetDirsLeft(int dl);
	void reset();

	Ui_ScanProgressBase *ui;
};
#endif
