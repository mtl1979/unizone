#include <qstatusbar.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qstring.h>

class WStatusBar : public QStatusBar
{
public:
	WStatusBar( QWidget * parent = 0, const char * name = 0 );
	~WStatusBar();

	void setText(QString text, int index);
	QString text(int index);
private:
	QLabel ** fText;
};