#ifndef SEARCH_H
#define SEARCH_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qdialog.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <qstatusbar.h>

#include "netclient.h"
#include "regex/StringMatcher.h"

#include <map>
using std::multimap;
using std::pair;
using std::iterator;

struct WFileInfo;	// private

class WSearch : public QDialog
{
	Q_OBJECT

public:
	WSearch(NetClient * net, QWidget * parent = NULL);
	virtual ~WSearch();

	void Lock() { fLock.lock(); }
	void Unlock() { fLock.unlock(); }

	void SetSearch(QString pattern);

signals:
	void Closed();		// the search window has been closed

protected:
	virtual void resizeEvent(QResizeEvent *);

private slots:
	void Close();
	void GoSearch();
	void StopSearch();
	void ClearList();
	void Download();
	void DisconnectedFromServer();

	void AddFile(const QString, const QString, bool, MessageRef);
	void RemoveFile(const QString, const QString);

private:
	NetClient * fNet;
	QVGroupBox * fMainBox;
	QVGroupBox * fSearchBox;
	QHGroupBox * fEntryBox;
	QHGroupBox * fButtonsBox;
	QLabel * fSearchLabel;
	QLineEdit * fSearchEdit;
	QListView * fSearchList;
	QPushButton * fDownload;
	QPushButton * fClose;
	QPushButton * fClear;
	QPushButton * fStop;
	QStatusBar * fStatus;
	QMutex fLock;		// to lock the list so only one method can be using it at a time

	QString fCurrentSearchPattern;
	StringMatcher fFileRegExp, fUserRegExp;
	QString fFileRegExpStr, fUserRegExpStr;

	bool fIsRunning;

	typedef multimap<QString, WFileInfo *> WFIMap;
	typedef pair<QString, WFileInfo *> WFIPair;
	typedef WFIMap::iterator WFIIter;

	WFIMap fFileList;

	void StartQuery(QString sidRegExp, QString fileRegExp);
	void SetResultsMessage();
	void SetStatus(const QString & status);

	// quick inline method to generate a pair
	WFIPair MakePair(const QString s, WFileInfo * fi)
	{
		WFIPair p;
		p.first = s;
		p.second = fi;
		return p;
	}
};




#endif	// SEARCH_H
