#include <qstatusbar.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qstring.h>

class WStatusBar : public QStatusBar
{
public:
	WStatusBar( QWidget * parent = 0, const char * name = 0, unsigned int columns = 3);
	~WStatusBar();

	void setText(QString text, unsigned int index);
	QString text(unsigned int index);
private:
	QLabel ** fText;
	unsigned int fColumns;
};