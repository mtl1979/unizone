#include "htmlview.h"
#include "tokenizer.h"
#include "wstring.h"
#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

#include "debugimpl.h"

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
#ifdef DEBUG2
	PRINT("WHTMLView: Press\n");
#endif
}

void
WHTMLView::viewportMouseReleaseEvent(QMouseEvent * e)
{
	QTextBrowser::viewportMouseReleaseEvent(e);
#ifdef DEBUG2
	PRINT("WHTMLView: Release\n");
#endif
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
#ifdef DEBUG2
	PRINT("WHTMLView: Move\n");
#endif
}

void 
WHTMLView::showEvent(QShowEvent * event)
{
	QTextBrowser::showEvent(event);
}

void 
WHTMLView::URLSelected(const QString & url)
{
	fURL = url;
	WString wURL(url);
	PRINT("WHTMLView: URLSelected: %S\n", wURL.getBuffer());
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
	WString wContext(fContext);
	PRINT("WHTMLView: setSource: %S\n", wContext.getBuffer());
	QTextBrowser::setSource( fContext );
	emit URLClicked( fContext );
}

QString
ParseForShown(const QString & txt)
{
	// <postmaster@raasu.org> 20021005,20021128 -- Don't use latin1(), use QStringTokenizer ;)
	QString out;
#ifdef DEBUG2									// Makes huge debug logs, so use only if really necessary
	QString temp = txt;
	temp.replace(QRegExp("<br>"), "\t");		// Resplit lines, so debugging is easier ;)
	QStringTokenizer tk(temp, "\t");
	QString next;
	QString line;
	while ((next = tk.GetNextToken()) != QString::null)
	{
		if (!next.isEmpty())
		{
			line = next;
			line += "<br>";	// replace our TAB
		}
		else
		{
			line = "<br>";	// replace our TAB
		}
		WString wLine(line);
		PRINT("ParseForShown: %S", wLine.getBuffer());	
		out += line;
	}
#else
	out = "";	// Text after processing linefeeds and TABs
	int n = 0;  // Position of next TAB before text to be included
	int n2 = 0;	// Start of actual text
	int m = 0;	// Position of next TAB after text to be included

	// Remove any extra line breaks from the start of buffer
	//

	while (n2 < (int) txt.length())
	{
		if (txt.mid(n2, 4) == "<br>")
		{
			n2 = n2 + 4;
		}
		else if (txt.mid(n2, 1) == "\t")
		{
			n2++;
		}
		else
		{
			break; // Found start of actual text
		}
	}

	n = txt.find('\t', n2);

	if (n > n2)
	{
		// copy everything before first TAB (after any extra line breaks stripped from the beginning)
		out += txt.mid(n2, n - n2);
		out += "<br>";

		// skip the TAB ;)
		n++;

		while (n < (int) txt.length())
		{
			m = txt.find('\t', n);
			if (m > n)
			{
				out += txt.mid(n, m - n);
				out += "<br>";
				n = m + 1;
			}
			else
			{
				// no more TAB characters
				out += txt.mid(n);
				n = txt.length();
			}
		}
	}
	else
	{
		out = txt.mid(n2);
	}
#endif

	// <postmaster@raasu.org> 20030721 -- Strip off trailing line break, we don't need it
	if (out.right(4) == "<br>")
		out.truncate(out.length() - 4);
	return out;
}

