#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qapplication.h>
#include <qtextbrowser.h>

#include "message/Message.h"

class muscle::Mutex;

using namespace muscle;

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

signals:
	void URLClicked(const QString & url);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);

	virtual void showEvent(QShowEvent * event);
	virtual void hideEvent(QHideEvent * event);

	virtual void timerEvent(QTimerEvent * event);

private:
	QString fOldURL, fURL, fContext;
	QString fBuffer;
	int fScrollX, fScrollY;				// do we need to scroll the view down after an insertion?
	bool fScrollDown;

	Mutex fLock;

	void CheckScrollState();
	void UpdateScrollState();

	void _append( const QString & text);

private slots:
	void URLSelected(const QString & url);
};

#endif
