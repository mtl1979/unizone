#ifndef PRIVATEWINDOW_H
#define PRIVATEWINDOW_H

#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include "privatewindow.h"
#include "chattext.h"
#include "user.h"
#include "htmlview.h"
#include "netclient.h"
#include "Log.h"

#include <qlistview.h>
#include <qsplitter.h>
#include <qvgroupbox.h>
#include <qthread.h>

// <postmaster@raasu.org> 20021024 -- Use always on Windows


class WPrivateWindow : public WPrivateWindowBase
{ 
    Q_OBJECT

public:
    WPrivateWindow(QObject * owner, NetClient * net, QWidget* parent = 0, 
					const char* name = 0, bool modal = false, WFlags fl = WDestructiveClose | WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_Minimize | WStyle_Maximize | WStyle_SysMenu);
		// the window will destroy itself when closed... and the destructor will throw a
		// WPWTextEvent with itself as the "SendTo" target. Be aware, this pointer is invalid!
    virtual ~WPrivateWindow();

	// received text from one of the users..
	void PutChatText(QString fromsid, QString msg);

	void AddUser(WUserRef & user);
	bool RemUser(WUserRef & user);
	WUserMap & GetUsers() { return fUsers; }

	void ClearBuffer() { fChat->ClearBuffer(); }

	void Lock() { fLock.lock(); }
	void Unlock() { fLock.unlock(); }

public slots:
	void UpdateUserList();

protected:
	virtual void customEvent(QCustomEvent * event);
	virtual void resizeEvent(QResizeEvent * e);

private slots:
	void URLSelected(const QString &);	// mouse over url
	void URLClicked();					// url clicked
	void UserDisconnected(QString sid, QString name);	// we need to remove user from internal list
	void DisconnectedFromServer();
	void TabPressed(QString str);

	// this won't be emitted under Windows
	void GotShown(const QString &);

	// popup menu
	void RightButtonClicked(QListViewItem *, const QPoint &, int);
	void PopupActivated(int);


private:
	mutable QMutex fLock;
	QObject * fOwner;
	NetClient * fNet;
	QListView * fPrivateUsers;
	WHTMLView * fText;	// chat text...
	WChatText * fChat;	// input box;
	QSplitter * fSplit;
	QSplitter * fSplitChat;
	QPopupMenu * fPopup;

	QString fURL;
	QString fPopupUser;
	WUserMap fUsers;	// users in list
	bool fScrollDown;
	WLog fLog;

#ifdef WIN32
	HWND fWinHandle;
#endif

	void UpdateView();
	void PrintText(const QString & str);
	void PrintError(const QString & error);
	void PrintSystem(const QString & msg);

	void CheckScrollState();
	void StartLogging();
	void StopLogging();

	friend class WinShareWindow;
};

typedef map<WPrivateWindow *, WPrivateWindow *> WPrivMap;
typedef WPrivMap::iterator WPrivIter;
typedef pair<WPrivateWindow *, WPrivateWindow *> WPrivPair;

inline WPrivPair
MakePair(WPrivateWindow * t)
{
	WPrivPair p;
	p.first = t;
	p.second = t;
	return p;
}

#endif // PRIVATEWINDOW_H
