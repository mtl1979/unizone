#ifndef ABOUTDLGIMPL_H
#define ABOUTDLGIMPL_H

#include <qdialog.h>

class AboutDlg : public QDialog
{
    Q_OBJECT

public:
	AboutDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WindowFlags fl = 0 );
    ~AboutDlg();

};

#endif // ABOUTDLG_H
