#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include "debugimpl.h"

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

signals:
	void URLClicked(const QString & url);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);
	
	virtual void showEvent(QShowEvent * event);

	virtual void BeforeShown();
	virtual void GotShown(const QString & txt);

private:
	QString fOldURL, fURL, fContext;
	int fScrollX, fScrollY;				// do we need to scroll the view down after an insertion?
	bool fScrollDown;

	void CheckScrollState();
	void UpdateTextView();
	void UpdateScrollState();

private slots:
	void URLSelected(const QString & url);
};

#endif


