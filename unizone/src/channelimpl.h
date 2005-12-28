#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qapplication.h>
#include <qsplitter.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>

#include "channel.h"
#include "chattext.h"
#include "chatwindow.h"
#include "htmlview.h"
#include "user.h"
#include "Log.h"

class NetClient;

class Channel : public ChannelBase, public ChatWindow
{ 
    Q_OBJECT

public:
    Channel( QWidget* parent = 0, NetClient * net = 0, QString cname = QString::null, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~Channel();
	void SetOwner(const QString &);
	void SetTopic(const QString &);
	void SetActive(bool);
	void SetPublic(bool);
	void AddUser(const QString & user);
	bool RemUser(const QString & user);
	void Invite(const QString & user);
	void Kick(const QString & user);
protected:
	virtual void customEvent(QCustomEvent *);
	virtual void resizeEvent(QResizeEvent * e);

	friend class Channels;
	void StartLogging();
	void StopLogging();

private:
	NetClient * fNet;
	QString fTopic, fOwner, fName, fStrAdmins;
	QSplitter * fSplit;
	QSplitter * fSplitBottom;
	QSplitter * fSplitChat;
	QHGroupBox * fTopicBox;
	QLabel * fTopicLabel;
	QLineEdit * fTopicEdit;
	QListView * fChannelUsers;
	WChatText * fInputText;
	QWidget * fParent;

	WUserMap fUsers;
	WUserMap fAdmins;
	bool fActive, fPublic;

	WLog fLog;

	void SendChannelText(const QString & message);

	void UpdateNode();

	WUserRef FindUser(const QString & user);

	void LogString(const char *);
	void LogString(const QString &);

	QWidget *Window();

public slots:
	void TabPressed(const QString &);
	void URLClicked(const QString &);
	void UpdateTopic();
	void NewChannelText(const QString &, const QString &, const QString &);
	void ChannelAdminsChanged(const QString &, const QString &);
	void UserDisconnected(const WUserRef &);

};

#endif // CHANNELIMPL_H
