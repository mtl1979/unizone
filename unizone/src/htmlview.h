#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qtextbrowser.h>

class WHTMLView : public QTextBrowser
{
	Q_OBJECT
public:
	WHTMLView(QWidget * parent = NULL, const char * name = NULL);
	virtual ~WHTMLView() {}

	virtual void setSource( const QString & name )	{}

signals:
	void URLClicked();
	void GotShown(const QString & txt);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);
	
#ifndef WIN32
	virtual void showEvent(QShowEvent * event);
#endif

private:
	QString fOldURL, fURL;

private slots:
	void URLSelected(const QString & url);
};

#endif


