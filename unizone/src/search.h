#ifndef SEARCH_H
#define SEARCH_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif


#include <qdialog.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>
#include <map>

#include "user.h"
#include "combo.h"
#include "ulistview.h"
#include "wstatusbar.h"
#include "searchitem.h"
#include "textevent.h"
#include "message/Message.h"
#include "support/MuscleSupport.h"

using namespace muscle;
using std::map;
using std::multimap;
using std::pair;

class NetClient;

struct WFileInfo
{
	WUserRef fiUser;
	QString fiFilename;
	uint64 fiSize;
	MessageRef fiRef;
	bool fiFirewalled;
	WSearchListItem * fiListItem;	// the list view item this file is tied to
};

typedef multimap<QString, WFileInfo *> WFIMap;
typedef pair<QString, WFileInfo *> WFIPair;
typedef WFIMap::iterator WFIIter;

class WSearch : public QDialog
{
	Q_OBJECT
public:
	WSearch(QWidget * parent, NetClient * fNet);
	virtual ~WSearch();

	void Cleanup();

	void StartQuery(const QString & sidRegExp, const QString & fileRegExp);
	int SplitQuery(const QString &fileExp);
	void SetResultsMessage();
	void SetSearchStatus(const QString & status, int index = 0);
	void SetSearch(const QString & pattern);

//	void QueueDownload(const QString & file, const WUserRef & user);
//	void EmptyQueues();

	void LoadSettings();
	void SaveSettings();

	bool GotResults() 
	{
		return fGotResults;
	}
protected:
	friend class WinShareWindow;

	void SetGotResults(bool b) 
	{
		fGotResults = b;
	}
	
private:
	mutable Mutex fSearchLock;		// to lock the list so only one method can be using it at a time

	uint64 fQueryBytes;

	bool fIsRunning, fGotResults;

	QGridLayout * fSearchTab;

	// Search Pane

	QLabel * fSearchLabel;
	WComboBox * fSearchEdit;
	WUniListView * fSearchList;
	QPushButton * fDownload;
	QPushButton * fClear;
	QPushButton * fStop;
	QPushButton * fClearHistory;
	WStatusBar * fStatus;

	QString fCurrentSearchPattern;
	QRegExp fFileRegExp, fUserRegExp;
	bool fFileRegExpNeg, fUserRegExpNeg;
	QString fFileRegExpStr, fUserRegExpStr;

	WFIMap fFileList;

	NetClient * fNetClient;

	void HandleComboEvent(WTextEvent * e);
	void customEvent(QCustomEvent * qce);

public slots:
	void StopSearch();

private slots:
	void AddFile(const QString &, const QString &, bool, MessageRef);
	void RemoveFile(const QString &, const QString &);

	void GoSearch();
	void ClearList();
	void Download();
	void ClearHistory();

};
#endif
