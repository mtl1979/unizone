#ifndef PRIVATEWINDOWIMPL_H
#define PRIVATEWINDOWIMPL_H

#include <qdialog.h>
#include <qlistview.h>
#include <q3popupmenu.h>
#include <qsplitter.h>
#include <q3vgroupbox.h>

#include "chattext.h"
#include "user.h"
#include "htmlview.h"
#include "Log.h"
#include "chatwindow.h"


class NetClient;

class WPrivateWindow : public ChatWindow
{
    Q_OBJECT

public:
    WPrivateWindow(QObject * owner, NetClient * net, QWidget* parent = 0,
		const char* name = 0,
		Qt::WindowFlags fl = Qt::WDestructiveClose | Qt::WStyle_Customize |
		Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_Minimize |
		Qt::WStyle_Maximize | Qt::WStyle_SysMenu);
		// the window will destroy itself when closed... and the destructor will throw a
		// WPWTextEvent with itself as the "SendTo" target. Be aware, this pointer is invalid!
    virtual ~WPrivateWindow();

	// received text from one of the users..
	void PutChatText(const QString & fromsid, const QString & msg);

	void AddUser(const WUserRef & user);
	void AddUsers(const WUserMap & umap);
	bool RemUser(const WUserRef & user);
	WUserMap & GetUsers() { return fUsers; }

	void ClearBuffer() { fInputText->ClearBuffer(); }

	void Lock() { fLock.Lock(); }
	void Unlock() { fLock.Unlock(); }

protected:
	virtual void customEvent(QEvent *);
	virtual void resizeEvent(QResizeEvent *);

private slots:
	void URLClicked(const QString &);	// url clicked
	void UserDisconnected(const WUserRef &);	// we need to remove user from internal list
	void DisconnectedFromServer();
	void TabPressed(const QString &);

	// popup menu
	void RightButtonClicked(Q3ListViewItem *, const QPoint &, int);
	void PopupActivated(int);


private:
	mutable Mutex fLock;
	QObject * fOwner;
	NetClient * fNet;
	Q3ListView * fPrivateUsers;
	WChatText * fInputText;	// input box;
	QSplitter * fSplit;
	QSplitter * fSplitChat;
	Q3PopupMenu * fPopup;

	QString fPopupUser;
	WUserMap fUsers;	// users in list
	WLog fLog;

	bool fEncrypted;

	void StartLogging();
	void StopLogging();

	void CheckEmpty();

	friend class WinShareWindow;

	void LogString(const char *);
	void LogString(const QString &);
	QWidget *Window();
};

#endif // PRIVATEWINDOW_H
