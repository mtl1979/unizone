#include "htmlview.h"
#include "tokenizer.h"
#include "wstring.h"
#include "debugimpl.h"
#include "parser.h"
#include "wmessageevent.h"

#include <qapplication.h>
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
	fScrollY = -1;
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
WHTMLView::showEvent(QShowEvent *)
{
#if (QT_VERSION < 0x030000)
//	Force scrolling down...
	fScrollDown = true;
	fScrollY = 0;
//  -----------------------
	QString txt;
	fLock.Lock();
	if (fBuffer.length() > 0)
	{
		txt = ParseForShown(fBuffer);
		fBuffer = "";
	}
	else
		txt = ParseForShown(text());
	setText("");
	setText(txt);
	fLock.Unlock();
	UpdateScrollState();
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
		if (_URL.find("#") > 0)
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
WHTMLView::append(const QString &newtext)
{
	PRINT("WHTMLView::append()\n");
#if (QT_VERSION < 0x030000)
	fLock.Lock();
	if (text().length() == 0)
	{
		setText(newtext);
	}
	else
	{
		QString tmp("\t");
		tmp += newtext;
		QTextBrowser::append(tmp);
	}
	fLock.Unlock();
#else
	fLock.Lock();
	QTextBrowser::append(newtext);
	fLock.Unlock();
#endif
	PRINT("WHTMLView::append() OK\n");
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
			if (fBuffer.length() == 0)
			{
				fBuffer = text();
			}
			setText("");
			PRINT("appendText 3\n");
			if (fBuffer.length() > 0)
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
		sendMessage(CheckMessage);
		PRINT("appendText: Calling append(fBuffer)\n");
		if (fBuffer.length() > 0)
		{
			MessageRef mref = GetMessageFromPool();
			if (mref())
			{
				mref()->AddString("text", (const char *) fBuffer.utf8());
				sendMessage(AppendMessage, mref);
			}
			fBuffer = "";
		}
		PRINT("appendText: Calling append(newtext)\n");
		MessageRef mref = GetMessageFromPool();
		if (mref())
		{
			mref()->AddString("text", (const char *) newtext.utf8());
			sendMessage(AppendMessage, mref);
		}
		PRINT("appendText: Calling UpdateScrollState()\n");
		sendMessage(ScrollMessage);
	}
	PRINT("appendText OK\n");
}

void
WHTMLView::clear()
{
	PRINT("WHTMLView::clear()\n");
	fLock.Lock();
	fBuffer = "";
	fScrollY = -1;
	setText("");
	fLock.Unlock();
	PRINT("WHTMLView::clear() OK\n");
}

void
WHTMLView::CheckScrollState()
{
	QScrollBar * scroll = verticalScrollBar();
#ifdef DEBUG2
	PRINT("CheckScrollState: ContentsX = %d, ContentsY = %d\n", fChatText->contentsX(),		fChatText->contentsY());
	PRINT("CheckScrollState: ContentsW = %d, ContentsH = %d\n", fChatText->contentsWidth(),	fChatText->contentsHeight());
	PRINT("CheckScrollState: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
#endif
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
WHTMLView::sendMessage(int type, MessageRef msg)
{
	WMessageEvent *wme = new WMessageEvent(type, msg);
	if (wme)
		QApplication::postEvent(this, wme);
}

void
WHTMLView::sendMessage(int type)
{
	WMessageEvent *wme = new WMessageEvent(type);
	if (wme)
		QApplication::postEvent(this, wme);
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
#ifndef WIN32	// linux only... (FreeBSD, QNX???)
		repaintContents(
							contentsX(), contentsY(),
							contentsWidth(), contentsHeight(),
							false);
#endif
		if (fScrollY <= contentsHeight())
			fScrollY = -1;
	}
}

bool
WHTMLView::event(QEvent *event)
{
	if (event->type() == (int) WMessageEvent::MessageEventType)
	{
		WMessageEvent *wme = dynamic_cast<WMessageEvent *>(event);
		if (wme)
		{
			if (wme->MessageType() == CheckMessage)
			{
				CheckScrollState();
				return true;
			}
			else if (wme->MessageType() == AppendMessage)
			{
				const char *text;
				MessageRef mref = wme->Message();
				mref()->FindString("text", &text);
				QString qtext = QString::fromUtf8(text);
				append(qtext);
				return true;
			}
			else if (wme->MessageType() == ScrollMessage)
			{
				UpdateScrollState();
				return true;
			}
		}
	}
	return QTextBrowser::event(event);
}
