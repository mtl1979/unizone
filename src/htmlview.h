#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qapplication.h>
#include <q3textbrowser.h>

#include "message/Message.h"
#include "system/Mutex.h"

using namespace muscle;

class WHTMLView : public Q3TextBrowser
{
	Q_OBJECT
public:
	WHTMLView(QWidget * parent = NULL, const char * name = NULL);
	virtual ~WHTMLView() {}

	virtual void setSource(const QString & name);
	virtual void appendText(const QString & text);
	virtual QString context() const {return fContext;}
	virtual void clear();

signals:
	void URLClicked(const QString & url);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);

	virtual void showEvent(QShowEvent * e);
	virtual void hideEvent(QHideEvent * e);

	virtual void timerEvent(QTimerEvent * e);

private:
	QString fOldURL, fURL, fContext;
	QString fBuffer;
	int fScrollX, fScrollY;				// do we need to scroll the view down after an insertion?
	bool fScrollDown;

	Mutex fLock;

	void CheckScrollState();
	void UpdateScrollState();

	void _append(const QString & text);

private slots:
	void URLSelected(const QString & url);
};

#endif
