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
protected:
	virtual void resizeEvent(QResizeEvent * e);
private:
	QLabel ** fText;
	QString * fLabels;
	unsigned int fColumns;

	QString checkText(const QString & text, unsigned int index);
};