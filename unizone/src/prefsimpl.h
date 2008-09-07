#ifndef PREFSIMPL_H
#define PREFSIMPL_H

#include "prefs.h"
#include "colors.h"

class WPrefs : public QDialog
{ 
    Q_OBJECT

public:
	WPrefs( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WindowFlags fl = 0 );
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

	Ui_WPrefsBase *ui;
};

#endif // PREFS_H
