#include "htmlview.h"
#include "tokenizer.h"
#include "wstring.h"
#include "debugimpl.h"
#include "parser.h"

#include <qtooltip.h>

#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

WHTMLView::WHTMLView(QWidget * parent, const char * name)
: QTextBrowser(parent, name)
{
	if (!name)
		setName( "WHTMLView" );
	fURL = fOldURL = fContext = QString::null;
	fScrollDown = true;
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
	BeforeShown();
	QString txt = text();
	setText("");
	GotShown(txt);
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

void
WHTMLView::appendText(const QString &newtext)
{
	PRINT("appendText\n");
	PRINT("appendText 1\n");
	BeforeShown();
	QWidget *widget = topLevelWidget();
	if (widget)
	{
		if (!widget->isVisible())
		{
#if (QT_VERSION >= 0x030000)
			int newlen = text().length() + newtext.length();
			if (newlen >= ParseBufferSize())
			{
#endif
				PRINT("appendText 2\n");
				QString temp = text();
				PRINT("appendText 3\n");
				setText("");
#if (QT_VERSION >= 0x030000)
				temp += "<br>";
#endif
				PRINT("appendText 4\n");
				temp += newtext;
				PRINT("appendText 5\n");
#if (QT_VERSION >= 0x030000)
				setText(ParseForShown(temp));
				PRINT("appendText: Calling GotShown()\n");
#endif
				GotShown(temp);
				PRINT("appendText OK\n");
				return;
#if (QT_VERSION >= 0x030000)
			}
#endif
		}
	}
	PRINT("appendText: Calling append()\n");
	append(newtext);
	PRINT("appendText: Calling UpdateTextView()\n");
	UpdateTextView();
	PRINT("appendText OK\n");
}

void
WHTMLView::BeforeShown()
{
	CheckScrollState();
}

void
#if (QT_VERSION < 0x030000)
WHTMLView::GotShown(const QString & txt)
#else
WHTMLView::GotShown(const QString &)
#endif
{
#if (QT_VERSION < 0x030000)
	setText(ParseForShown(txt));
#endif
	UpdateTextView();
}

void
WHTMLView::CheckScrollState()
{
	QScrollBar * scroll = verticalScrollBar();
#ifdef DEBUG2
	PRINT("CHECKSCROLLSTATE: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
#endif
	fScrollX = contentsX();
	fScrollY = contentsY();
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}

void
WHTMLView::UpdateTextView()
{
	// <postmaster@raasu.org> 20021021 -- Fixed too long line in debug output
#ifdef DEBUG2
	PRINT("UPDATETEXTVIEW: ContentsX = %d, ContentsY = %d\n", fChatText->contentsX(),		fChatText->contentsY());
	PRINT("              : ContentsW = %d, ContentsH = %d\n", fChatText->contentsWidth(),	fChatText->contentsHeight());
#endif
	UpdateScrollState();
}

void
WHTMLView::UpdateScrollState()
{
	if (fScrollDown)
	{
		fScrollY = contentsHeight();
	}
	if (fScrollX != contentsX() || fScrollY != contentsY())
	{
		setContentsPos(fScrollX, fScrollY);
#ifndef WIN32	// linux only... (FreeBSD???)
		repaintContents(
							contentsX(), contentsY(),
							contentsWidth(), contentsHeight(),
							false);
#endif
	}
}
