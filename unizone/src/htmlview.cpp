#include "htmlview.h"
#include "debugimpl.h"

#include <qtooltip.h>

WHTMLView::WHTMLView(QWidget * parent, const char * name)
		: QTextBrowser(parent, name)
{
	if (!name)
		setName( "WHTMLView" );
	fURL = fOldURL = QString::null;
	connect( this, SIGNAL(highlighted(const QString &)), this, SLOT(URLSelected(const QString &)) );
}

void
WHTMLView::viewportMousePressEvent(QMouseEvent * e)
{
	emit URLClicked();
	QTextBrowser::viewportMousePressEvent(e);
	PRINT("WHTMLView: Press\n");
}

void
WHTMLView::viewportMouseReleaseEvent(QMouseEvent * e)
{
	QTextBrowser::viewportMouseReleaseEvent(e);
	PRINT("WHTMLView: Release\n");
}

void
WHTMLView::viewportMouseMoveEvent(QMouseEvent * e)
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
void 
WHTMLView::showEvent(QShowEvent * event)
{
	QString txt = text();
	setText("");
	emit GotShown(txt);
}
#endif

void 
WHTMLView::URLSelected(const QString & url)
{
	fURL = url;
}

