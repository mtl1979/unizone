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

	virtual QString context() { return fContext; }

signals:
	void URLClicked(const QString & url);
	void BeforeShown();
	void GotShown(const QString & txt);

protected:
	virtual void viewportMousePressEvent(QMouseEvent * e);
	virtual void viewportMouseReleaseEvent(QMouseEvent * e);
	virtual void viewportMouseMoveEvent(QMouseEvent * e);
	
	virtual void showEvent(QShowEvent * event);

private:
	QString fOldURL, fURL, fContext;

private slots:
	void URLSelected(const QString & url);
};

// Parse text for showing on WHTMLView
QString ParseForShown(const QString & txt);
QString ParseForShownAux(const QString &txt);


#endif


