#include "htmlview.h"
#include "tokenizer.h"

#ifdef UNIVIEW
#  define PRINT qDebug
#else
#  include "debugimpl.h"
#endif

#include <qtooltip.h>

WHTMLView::WHTMLView(QWidget * parent, const char * name)
: QTextBrowser(parent, name)
{
	if (!name)
		setName( "WHTMLView" );
	fURL = fOldURL = fContext = QString::null;
	connect( this, SIGNAL(highlighted(const QString &)), this, SLOT(URLSelected(const QString &)) );
}

void
WHTMLView::viewportMousePressEvent(QMouseEvent * e)
{
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

void 
WHTMLView::showEvent(QShowEvent * event)
{
#ifdef UNIVIEW
	QTextBrowser::showEvent(event);
#else
	QString txt = text();
	setText("");
	emit GotShown(txt);
#endif
}

void 
WHTMLView::URLSelected(const QString & url)
{
	fURL = url;
	PRINT("WHTMLView: URLSelected: %s\n", url.latin1());
}

void 
WHTMLView::setSource( const QString & name )	
{
	QString _URL = fContext;
	fContext = name;
	if (name.at(0) == "#")
	{
		if (_URL.find("#") >= 0)
		{
			fContext = _URL.left(_URL.find("#"));
			fContext += name;
		}
	}
	PRINT("WHTMLView: setSource: %s\n", fContext.latin1());
#ifdef UNIVIEW
	QTextBrowser::setSource( fContext );
#endif
	emit URLClicked( fContext );
}

QString
ParseForShown(const QString & txt)
{
	// <postmaster@raasu.org> 20021005,20021128 -- Don't use latin1(), use QStringTokenizer ;)
	QStringTokenizer tk(txt, "\t");
	QString next;
	QString out;
	while ((next = tk.GetNextToken()) != QString::null)
	{
		next = next.stripWhiteSpace();
		if (!next.isEmpty())
			out += next; 
//#if (QT_VERSION < 0x030100)
		out += "<br>";	// replace our TAB
//#endif
	}
	out = out.stripWhiteSpace();

	// <postmaster@raasu.org> 20030722 -- Strip off preceding line breaks, we don't need them	
	while (out.left(4) == "<br>")
		out = out.mid(4);

	// <postmaster@raasu.org> 20030721 -- Strip off trailing line breaks, we don't need them
	while (out.right(4) == "<br>")
		out.truncate(out.length() - 4);
	qDebug("ParseForShown: %s", out.latin1());
	return out;
}

