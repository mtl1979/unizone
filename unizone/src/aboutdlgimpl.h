#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "aboutdlg.h"

class AboutDlg : public AboutDlgBase
{ 
    Q_OBJECT

public:
    AboutDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AboutDlg();

};

#endif // ABOUTDLG_H
