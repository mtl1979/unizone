#ifndef CHATTEXT_H
#define CHATTEXT_H

#include <qmultilineedit.h>

#include "util/Queue.h"
using namespace muscle;

/** This class will chat SHIFT+ENTER and ENTER keystrokes and
  * send the appropriate output to the text input
  */
class WChatText : public QMultiLineEdit
{
	Q_OBJECT
public:
	WChatText(QObject * target, QWidget * parent = NULL);
	virtual ~WChatText();

	void ClearBuffer();

	void gotoEnd();

signals:
	void TabPressed(const QString &str);

protected:
	virtual void keyPressEvent(QKeyEvent * event);

private:
	Queue<QString> * fBuffer;	// line buffer
	QObject * fTarget;
	int fCurLine;				// which line in the buffer?
};

#endif	// CHATEXT_H

