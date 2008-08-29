#ifdef WIN32
#pragma warning (disable: 4512)
#endif

#include <qtooltip.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QHideEvent>
#include <QMouseEvent>
#include <QTimerEvent>

#include "htmlview.h"
#include "tokenizer.h"
#include "wstring.h"
#include "debugimpl.h"
#include "parser.h"
#include "wmessageevent.h"

WHTMLView::WHTMLView(QWidget * parent, const char * name)
: Q3TextBrowser(parent, name)
{
	if (!name)
		setName( "WHTMLView" );
	fURL = fOldURL = fContext = QString::null;
	fScrollY = -1;
	fScrollDown = true;
	connect( this, SIGNAL(highlighted(const QString &)), this, SLOT(URLSelected(const QString &)) );
	startTimer(1000);
}

void
WHTMLView::viewportMousePressEvent(QMouseEvent * e)
{
	PRINT2("WHTMLView: Press\n");
	Q3TextBrowser::viewportMousePressEvent(e);
}

void
WHTMLView::viewportMouseReleaseEvent(QMouseEvent * e)
{
	PRINT2("WHTMLView: Release\n");
	Q3TextBrowser::viewportMouseReleaseEvent(e);
}

void
WHTMLView::viewportMouseMoveEvent(QMouseEvent * e)
{
	PRINT2("WHTMLView: Move\n");
	// Tooltip
	if (fOldURL != fURL)
	{
		fOldURL = fURL;
		QToolTip::remove(this);
		if (fURL != QString::null)
			QToolTip::add(this, fURL);
	}
	Q3TextBrowser::viewportMouseMoveEvent(e);
}

void
WHTMLView::hideEvent(QHideEvent * e)
{
	PRINT("WHTMLView::hideEvent()\n");
#if (QT_VERSION < 0x030000) && defined(DEBUG2)

	fLock.Lock();
	
	QString tmp;
	
	if (fBuffer.isEmpty())
	{
		fBuffer = TrimBuffer(text());
		setText(QString::null);
	}
	
	fLock.Unlock();
	
#endif
	Q3TextBrowser::hideEvent(e);
	PRINT("WHTMLView::hideEvent() OK\n");
}

void 
WHTMLView::showEvent(QShowEvent * e)
{
	PRINT("WHTMLView::showEvent()\n");
#if (QT_VERSION < 0x030000)

	fLock.Lock();

	QString txt;

//	Force scrolling down...
	fScrollDown = true;
	fScrollY = 0;
//  -----------------------
	if (fBuffer.isEmpty())
	{
		txt = text();
	}
	else
	{
#ifndef DEBUG2
		if (!text().isEmpty())
		{
			txt = text();
//			txt += "<br>";
		}
#endif
		txt += fBuffer;
		fBuffer = QString::null;
	}

	txt = TrimBuffer(txt);
	txt = ParseForShown(txt);
	setText(txt);

	fLock.Unlock();

	UpdateScrollState();
#endif
	Q3TextBrowser::showEvent(e);
	PRINT("WHTMLView::showEvent() OK\n");
}

void 
WHTMLView::URLSelected(const QString & url)
{
	fURL = url;
#ifdef _DEBUG
	if (!fURL.isEmpty())
	{
		WString wurl(url);
		PRINT("WHTMLView: URLSelected: %S\n", wurl.getBuffer());
	}
#endif
}

void 
WHTMLView::setSource( const QString & name )	
{
	QString _URL = fContext;
	fContext = name;
	if (name.at(0) == '#')
	{
		if (_URL.find("#") > 0)
		{
			fContext = _URL.left(_URL.find("#"));
			fContext += name;
		}
	}
#ifdef _DEBUG
	WString wctx(fContext);
	PRINT("WHTMLView: setSource: %S\n", wctx.getBuffer());
#endif
	emit URLClicked( fContext );
}

void
WHTMLView::_append(const QString &newtext)
{
	PRINT("WHTMLView::_append()\n");
	fLock.Lock();

#if (QT_VERSION < 0x030000)
	if (text().isEmpty())
	{
		PRINT("Calling setText()\n");
		setText(TrimBuffer(newtext));
	}
	else
	{
		QString tmp("\t");
		tmp += newtext;
		PRINT("Calling append()\n");
		append(tmp);
	}
#else
	append(newtext);
#endif

	fLock.Unlock();
	PRINT("WHTMLView::_append() OK\n");
}

void
WHTMLView::appendText(const QString &newtext)
{
	PRINT("appendText\n");

	QWidget *widget = topLevelWidget();
	if (widget)
	{
		PRINT("appendText 1\n");
		if (!widget->isVisible())
		{
			PRINT("appendText 2\n");
			// Synchronize
			fLock.Lock();
			CheckScrollState();	
#ifdef DEBUG2
			if (fBuffer.isEmpty())
			{
				fBuffer = text();
				if (!fBuffer.isEmpty())
					setText(QString::null);
			}
#endif
			PRINT("appendText 3\n");
			if (!fBuffer.isEmpty())
			{
				fBuffer += "<br>";
			}
			PRINT("appendText 4\n");
			fBuffer += newtext;
/*	We don't use UpdateScrollState() here because nothing is really shown on the screen */
			PRINT("appendText OK\n");
			fLock.Unlock();
			return;
		}
	}
	// fall through here...
	{
		PRINT("appendText: Calling CheckScrollState()\n");
		CheckScrollState();

		// Use temporary variable to minimize time being locked.
		QString tmp;
		fLock.Lock();
		if (!fBuffer.isEmpty())
		{
			tmp = fBuffer;
			fBuffer = QString::null;
		}
		fLock.Unlock();

		if (!tmp.isEmpty())
		{
			tmp += "<br>";
			tmp += newtext;
		}
		else
			tmp = newtext;

		PRINT("appendText: Calling _append(tmp)\n");
		_append(tmp);
		PRINT("appendText: Calling UpdateScrollState()\n");
		UpdateScrollState();
	}
	PRINT("appendText OK\n");
}

void
WHTMLView::clear()
{
	PRINT("WHTMLView::clear()\n");
	fLock.Lock();
	fBuffer = QString::null;
	fScrollY = -1;
	setText(QString::null);
	fLock.Unlock();
	PRINT("WHTMLView::clear() OK\n");
}

void
WHTMLView::CheckScrollState()
{
	QScrollBar * scroll = verticalScrollBar();
	PRINT2("CheckScrollState: ContentsX = %d, ContentsY = %d\n", contentsX(),		contentsY());
	PRINT2("CheckScrollState: ContentsW = %d, ContentsH = %d\n", contentsWidth(),	contentsHeight());
	PRINT2("CheckScrollState: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
	fScrollX = contentsX();
	if (scroll->value() >= scroll->maxValue())
	{
		fScrollDown = true;
	}
	else
	{
		fScrollDown = false;
		if (fScrollY == -1)
			fScrollY = scroll->value();
	}
}

void
WHTMLView::UpdateScrollState()
{
	// if we don't have previous state saved, let's scroll to bottom
	if (fScrollY == -1)
		fScrollDown = true;
	// -------------------------------------------------------------

	if (fScrollDown)
		fScrollY = contentsHeight();

	if (fScrollX > contentsX() || fScrollY > contentsY())
	{
		setContentsPos(fScrollX, QMIN(fScrollY, contentsHeight()));
#if (QT_VERSION < 0x030000)
		repaintContents(
							contentsX(), contentsY(),
							contentsWidth(), contentsHeight(),
							false);
#endif
		if (fScrollY <= contentsHeight())
			fScrollY = -1;
	}
}

void
WHTMLView::timerEvent(QTimerEvent *)
{
	// Workaround for skipped QShowEvents
	fLock.Lock();
	if (!fBuffer.isEmpty())
	{
		QWidget *widget = topLevelWidget();
		if (widget && widget->isVisible())
		{
			CheckScrollState();
			_append(fBuffer);
			fBuffer = QString::null;
			UpdateScrollState();
		}
	}
	fLock.Unlock();
}
