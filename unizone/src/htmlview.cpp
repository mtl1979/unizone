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
	QString txt;
	if (fBuffer.length() > 0)
	{
		txt = fBuffer;
		fBuffer = "";
	}
	else
		txt = text();
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
WHTMLView::append(const QString &text)
{
	PRINT("WHTMLView::append()\n");
#if (QT_VERSION < 0x030000)
	QString tmp("\t");
	tmp += text;
	QTextBrowser::append(tmp);
#else
	QTextBrowser::append(text);
#endif
	PRINT("WHTMLView::append() OK\n");
}

void
WHTMLView::appendText(const QString &newtext)
{
	PRINT("appendText\n");

	if (fBuffer.length() == 0)
		BeforeShown();	

	QWidget *widget = topLevelWidget();
	if (widget)
	{
		PRINT("appendText 1\n");
		if (!widget->isVisible())
		{
			PRINT("appendText 2\n");
			if (fBuffer.length() == 0)
			{
				fBuffer = text();
			}
			PRINT("appendText 3\n");
			setText("");
			if (fBuffer.length() > 0)
			{
#if (QT_VERSION >= 0x030000)
				fBuffer += "<br>";
#else
				fBuffer += "<br>";
#endif
			}
			PRINT("appendText 4\n");
			fBuffer += newtext;
			PRINT("appendText 5\n");
			GotShown(fBuffer);
			PRINT("appendText OK\n");
			return;
		}
	}
	// fall through here...
	{
		PRINT("appendText: Calling append()\n");
		if (fBuffer.length() > 0)
		{
			append(fBuffer);
			fBuffer = "";
		}
		append(newtext);
		PRINT("appendText: Calling UpdateTextView()\n");
		UpdateTextView();
	}
	PRINT("appendText OK\n");
}

void
WHTMLView::BeforeShown()
{
	CheckScrollState();
}

void
WHTMLView::GotShown(const QString & txt)
{
	if (fBuffer.length() == 0)
	{
		setText(ParseForShown(txt));
		UpdateTextView();
	}
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
