#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qtextbrowser.h>

#include "debugimpl.h"

class WHTMLView : public QTextBrowser
{
	Q_OBJECT
public:
	WHTMLView(QWidget * parent = NULL, const char * name = NULL) 
		: QTextBrowser(parent, name)
	{
		if (!name)
			setName( "WHTMLView" );
	}

	virtual ~WHTMLView() {}

	virtual void setSource( const QString & name )	{}

signals:
	void URLClicked();
	void GotShown(const QString & txt);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e)
	{
		emit URLClicked();
		QTextBrowser::viewportMousePressEvent(e);
		PRINT("WHTMLView: Press\n");
	}
	virtual void viewportMouseReleaseEvent(QMouseEvent * e)
	{
		QTextBrowser::viewportMouseReleaseEvent(e);
		PRINT("WHTMLView: Release\n");
	}
	
#ifndef WIN32
	virtual void showEvent(QShowEvent * event)
	{
		QString txt = text();
		setText("");
		emit GotShown(txt);
	}
#endif
};

#endif


