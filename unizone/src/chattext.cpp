#include "chattext.h"
#include "textevent.h"
#include "debugimpl.h"

#include <qapplication.h>

#define DEFAULT_SIZE 10

WChatText::WChatText(QObject * target, QWidget * parent)
	: QMultiLineEdit(parent), fBuffer(new Queue<QString>(DEFAULT_SIZE)), fTarget(target) 
{
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
//			if (fCurLine == (int)fBuffer->GetNumItems())		// don't add duplicate items
			if (text() != line)
			{
				fBuffer->AddTail(text());
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
		else
		{
			if (fCurLine > 0)
			{
				fCurLine--;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				setCursorPosition(9999, 9999);
			}
		}
	}
	else if (event->key() == Key_Down)
	{
		if (event->state() & AltButton)
		{
			QMultiLineEdit::keyPressEvent(event);
		}
		else
		{
			if (fCurLine < (int)fBuffer->GetNumItems() - 1)
			{
				fCurLine++;
				QString line;
				fBuffer->GetItemAt(fCurLine, line);
				setText(line);
				setCursorPosition(9999, 9999);
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
	fBuffer->Clear();
	fCurLine = 0;	// no more items
}

