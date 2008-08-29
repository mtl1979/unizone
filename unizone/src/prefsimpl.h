#ifndef PREFS_H
#define PREFS_H
#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "prefs.h"
#include "colors.h"

class WPrefs : public WPrefsBase
{ 
    Q_OBJECT

public:
	WPrefs( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~WPrefs();

private slots:
	void OK();
	void Cancel();
	void StyleSelected(int index);
	void ColorSelected(int index);
	void ChangeColor();
	void ChangeSound();
	void ResetSound();
	void AwaySelected(int index);

private:
	void UpdateDescription(int);
	
	int fCurColorIndex;
	QString fColor[WColors::NumColors];
};

#endif // PREFS_H
