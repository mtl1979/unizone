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
#include "Log.h"
#include "chatwindow.h"

#include <qlistview.h>
#include <qsplitter.h>
#include <qvgroupbox.h>
#include <qthread.h>


class NetClient;

class WPrivateWindow : public WPrivateWindowBase, public ChatWindow
{ 
    Q_OBJECT

public:
    WPrivateWindow(QObject * owner, NetClient * net, QWidget* parent = 0, 
					const char* name = 0, bool modal = false, WFlags fl = WDestructiveClose | WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_Minimize | WStyle_Maximize | WStyle_SysMenu);
		// the window will destroy itself when closed... and the destructor will throw a
		// WPWTextEvent with itself as the "SendTo" target. Be aware, this pointer is invalid!
    virtual ~WPrivateWindow();

	// received text from one of the users..
	void PutChatText(const QString & fromsid, const QString & msg);

	void AddUser(const WUserRef & user);
	bool RemUser(const WUserRef & user);
	WUserMap & GetUsers() { return fUsers; }

	void ClearBuffer() { fInputText->ClearBuffer(); }

	void Lock() { fLock.lock(); }
	void Unlock() { fLock.unlock(); }

public slots:
	void UpdateUserList();

protected:
	virtual void customEvent(QCustomEvent * event);
	virtual void resizeEvent(QResizeEvent * e);

private slots:
	void URLClicked(const QString &);	// url clicked
	void UserDisconnected(const QString &, const QString &);	// we need to remove user from internal list
	void DisconnectedFromServer();
	void TabPressed(const QString &);

	// popup menu
	void RightButtonClicked(QListViewItem *, const QPoint &, int);
	void PopupActivated(int);


private:
	mutable QMutex fLock;
	QObject * fOwner;
	NetClient * fNet;
	QListView * fPrivateUsers;
	WChatText * fInputText;	// input box;
	QSplitter * fSplit;
	QSplitter * fSplitChat;
	QPopupMenu * fPopup;

	QString fPopupUser;
	WUserMap fUsers;	// users in list
	WLog fLog;

	void StartLogging();
	void StopLogging();

	void CheckEmpty();

	friend class WinShareWindow;

	void LogString(const char *);
	void LogString(const QString &);
	QWidget *Window();
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
