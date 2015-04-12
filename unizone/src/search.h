#ifndef SEARCH_H
#define SEARCH_H

#include <qapplication.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>

#include "message/Message.h"
#include "qtsupport/QMuscleSupport.h"

#include "user.h"
#include "combo.h"
#include "ulistview.h"
#include "wstatusbar.h"
#include "searchitem.h"
#include "textevent.h"


using namespace muscle;

class NetClient;
class QString;

struct WFileInfo
{
	WUserRef fiUser;
	QString fiFilename;
	QString fiPath;
	uint64 fiSize;
	MessageRef fiRef;
	bool fiFirewalled;
	WSearchListItem * fiListItem;	// the list view item this file is tied to
};

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

	void SetSearchPassive();

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
	QPushButton * fDownloadAll;
	QPushButton * fClear;
	QPushButton * fStop;
	QPushButton * fClearHistory;
	WStatusBar * fStatus;

	QString fCurrentSearchPattern;
	QRegExp fFileRegExp, fUserRegExp;
	bool fFileRegExpNeg, fUserRegExpNeg;
	QString fFileRegExpStr, fUserRegExpStr;

	Hashtable<QString, WFileInfo *> fFileList;

	NetClient * fNetClient;

	bool fPending;

	void HandleComboEvent(WTextEvent *);
	void customEvent(QEvent *);

public slots:
	void StopSearch();

private slots:
	void ServerConnected();
	void ServerDisconnected();

	void AddFile(const WUserRef &, const QString &, bool, MessageRef);
	void RemoveFile(const WUserRef &, const QString &);

	void GoSearch();
	void ClearList();
	void Download();
	void DownloadAll();
	void ClearHistory();

};
#endif
