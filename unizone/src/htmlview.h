#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include "system/Mutex.h"
#include "message/Message.h"

using namespace muscle;

#include <qtextbrowser.h>

class WHTMLView : public QTextBrowser
{
	Q_OBJECT
public:
	WHTMLView(QWidget * parent = NULL, const char * name = NULL);
	virtual ~WHTMLView() {}

	virtual void setSource( const QString & name );	
	virtual void appendText( const QString & text);
	virtual QString context() const { return fContext; }
	virtual void clear();

	enum
	{
		CheckMessage = 'wHcM',
		AppendMessage,
		ScrollMessage
	};


signals:
	void URLClicked(const QString & url);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);
	
	virtual void showEvent(QShowEvent * event);

	virtual bool event(QEvent * event);

private:
	QString fOldURL, fURL, fContext;
	QString fBuffer;
	int fScrollX, fScrollY;				// do we need to scroll the view down after an insertion?
	bool fScrollDown;

	Mutex fLock;

	void CheckScrollState();
	void UpdateScrollState();

	virtual void append( const QString & text);
	void sendMessage(int type, MessageRef msg);
	void sendMessage(int type);

private slots:
	void URLSelected(const QString & url);
};

#endif
