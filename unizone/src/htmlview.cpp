#include "htmlview.h"
#include "tokenizer.h"
#include "wstring.h"
#include "debugimpl.h"

#include <qtooltip.h>

#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

#define MAX_BUFFER_SIZE 262144		// 256 kB

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
WHTMLView::showEvent(QShowEvent * /* event */)
{
#if (QT_VERSION < 0x030000)
	emit BeforeShown();
	QString txt = text();
	setText("");
	emit GotShown(txt);
#endif
}

void 
WHTMLView::URLSelected(const QString & url)
{
	fURL = url;
#ifdef _DEBUG
	WString wURL(url);
	PRINT("WHTMLView: URLSelected: %S\n", wURL.getBuffer());
#endif
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
#ifdef _DEBUG
	WString wContext(fContext);
	PRINT("WHTMLView: setSource: %S\n", wContext.getBuffer());
#endif
	emit URLClicked( fContext );
}

QString
ParseForShown(const QString & txt)
{
	// <postmaster@raasu.org> 20021005,20021128 -- Don't use latin1(), use QStringTokenizer ;)
	QString out;
	out = ParseForShownAux(txt);

	// <postmaster@raasu.org> 20030721 -- Strip off trailing line break, we don't need it
	if (out.right(4) == "<br>")
		out.truncate(out.length() - 4);
	return out;
}

QString
ParseForShownAux(const QString &txt)
{
	int n = 0;			// Position of next TAB before text to be included
	int n2 = 0;			// Start of actual text
	int m = 0;			// Position of next TAB after text to be included

	// Remove any extra line breaks from the start of buffer
	//

	// Check for too big text
	if (txt.length() > MAX_BUFFER_SIZE)
	{
		int n3;						// Position of next line break after buffer truncation
		n2 = txt.length() - MAX_BUFFER_SIZE;
		
		// Find next line break

		n3 = txt.find("<br>", n2);
		if (n3 < 0)
		{
			n3 = txt.find('\t', n2);
			if (n3 >= 0)
			{
				n2 = n3 + 1;
			}
		}
		else
		{
			n2 = n3 + 4;
		}
	}

	// Remove any extra line breaks from the start of buffer
	//

	while (n2 < (int) txt.length())
	{
		if (txt.mid(n2, 4) == "<br>")
		{
			n2 += 4;
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
		QString out;	// Text after processing linefeeds and TABs

		// copy everything before first TAB (after any extra line breaks stripped from the beginning)
		out = txt.mid(n2, n - n2);
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
				break;
			}
		}
		return out;
	}
	else
	{
		return txt.mid(n2);
	}
	return txt;
}

void
WHTMLView::appendText(const QString &newtext)
{
	PRINT("appendText\n");
	QWidget *widget = topLevelWidget();
	if (widget)
	{
		if (!widget->isVisible())
		{
			int newlen = text().length() + newtext.length();
//			if (newlen >= MAX_BUFFER_SIZE)
//			{
				PRINT("appendText 1\n");
				emit BeforeShown();
				PRINT("appendText 2\n");
				QString temp = text();
				PRINT("appendText 3\n");
				setText("");
				PRINT("appendText 4\n");
				temp += newtext;
#if (QT_VERSION >= 0x030000)
				PRINT("appendText 5\n");
				setText(ParseForShown(temp));
				PRINT("appendText 6\n");
#else
				PRINT("appendText 5\n");
#endif
				emit GotShown(temp);
				PRINT("appendText OK\n");
				return;
//			}
		}
	}
	append(newtext);
	PRINT("appendText OK\n");
}