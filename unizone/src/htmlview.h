#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qtextbrowser.h>
#include <qtooltip.h>

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
		fURL = fOldURL = QString::null;
		connect( this, SIGNAL(highlighted(const QString &)), this, SLOT(URLSelected(const QString &)) );
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

	virtual void viewportMouseMoveEvent(QMouseEvent * e)
	{
		if (fOldURL != fURL)
		{
			fOldURL = fURL;
			QToolTip::remove(this);
			if (fURL != QString::null)
				QToolTip::add(this, fURL);
		}
		QTextBrowser::viewportMouseMoveEvent(e);
		PRINT("WHTMLView: Move\n");
	}
	
#ifndef WIN32
	virtual void showEvent(QShowEvent * event)
	{
		QString txt = text();
		setText("");
		emit GotShown(txt);
	}
#endif
private:
	QString fOldURL, fURL;

private slots:
	void URLSelected(const QString & url)
	{
		fURL = url;
	}
};

#endif


