#include "chattext.h"
#include "textevent.h"
#include "debugimpl.h"

#include <qapplication.h>

#define DEFAULT_SIZE 10
#define MAX_SIZE 100

WChatText::WChatText(QObject * target, QWidget * parent)
	: QMultiLineEdit(parent), fTarget(target) 
{
	fBuffer = new Queue<QString>(DEFAULT_SIZE);
	fCurLine = 0;
}

WChatText::~WChatText()
{
	delete fBuffer;
}

void
WChatText::keyPressEvent(QKeyEvent * event)
{
	if (event->key() == Key_Return || event->key() == Key_Enter)    // check for BOTH (they ARE different to Qt)
	{
		if ((event->state() & ShiftButton))
		{
			newLine();	// new line if SHIFT+ENTER is pressed
		}
		else
		{
			// otherwise, send text
			WTextEvent *wte = new WTextEvent(text());
			if (wte)
			{
				if (!wte->Valid())
				{
					delete wte;
					wte = NULL;
					return;
				}
				QApplication::postEvent(fTarget, wte);			// <postmaster@raasu.org> 20021024 -- Fix Access violation due duplicate definition with different type
			}
			QString line;
			fBuffer->GetItemAt(fCurLine, line);
			
			// don't add duplicate items
			if (text() != line)
			{
				AddLine(text());
				PRINT("Lines %d\n", fCurLine);
			}
			fCurLine = fBuffer->GetNumItems();	// reset
			setText("");
		}
	}
	else if (event->key() == Key_Tab)
	{
		PRINT("Emitting TAB\n");
		// tab completion
		emit TabPressed(text());
	}	
	else if (event->key() == Key_Up)
	{
		if (event->state() & AltButton)
		{
			QMultiLineEdit::keyPressEvent(event);
		}
		else if (event->state() & ControlButton)				
		{
			// First line
			if (fCurLine > 0)
			{
				fCurLine = 0;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				gotoEnd();
			}
		}
		else
		{
			// Previous line
			if (fCurLine > 0)
			{
				fCurLine--;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				gotoEnd();
			}
		}
	}
	else if (event->key() == Key_Down)
	{
		if (event->state() & AltButton)
		{
			QMultiLineEdit::keyPressEvent(event);
		}
		else if (event->state() & ControlButton)					
		{
			// Last line
			if (fCurLine < (int)fBuffer->GetNumItems() - 2)
			{
				fCurLine = fBuffer->GetNumItems() - 1;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				gotoEnd();
			}
			else
			{
				fCurLine = fBuffer->GetNumItems();
				setText("");
			}
		}
		else
		{
			// Next line
			if (fCurLine < (int)fBuffer->GetNumItems() - 1)
			{
				fCurLine++;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				gotoEnd();
			}
			else
			{
				fCurLine = fBuffer->GetNumItems();
				setText("");
			}
		}
	}
	else if (event->key() == Key_Escape)
	{
		setText("");
		fCurLine = fBuffer->GetNumItems();
	}
	else
		QMultiLineEdit::keyPressEvent(event);
}

void
WChatText::ClearBuffer()
{
	fLock.Lock();
	fBuffer->Clear();
	fCurLine = 0;	// no more items
	fLock.Unlock();
}

void
WChatText::gotoEnd()
{
	int lastline = numLines() - 1;
	int len = lineLength(lastline);
	setCursorPosition(lastline, len);
}

void
WChatText::AddLine(const QString &line)
{
	QString junk;
	int l = 0;
	fLock.Lock();
	// Remove duplicate entries
	while (l < (int) fBuffer->GetNumItems())
	{
		fBuffer->GetItemAt(l, junk);
		if (junk == line)
		{
			fBuffer->RemoveItemAt(l);
			if (l <= fCurLine)
				fCurLine--;
		}
		else
		{
			l++;
		}

		if (l >= (int) fBuffer->GetNumItems())
			break;
	}
	// Remove enough old entries, so total amount after adding this line will not exceed MAX_SIZE
	while (fBuffer->GetNumItems() >= MAX_SIZE)
	{
		fBuffer->RemoveHead(junk);
		fCurLine--;
	}
	if (fCurLine < 0)
		fCurLine = 0;
	fBuffer->AddTail(line);
	fLock.Unlock();
}
