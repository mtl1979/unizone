#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qsplitter.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>

#include "channel.h"
//#include "netclient.h"
#include "chattext.h"
#include "htmlview.h"
#include "user.h"

class NetClient;

class Channel : public ChannelBase
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
	WChatText * fChat;
	WHTMLView * fText;
	QWidget * fParent;

	WUserMap fUsers;
	WUserMap fAdmins;
	bool fScrollDown;
	bool fActive, fPublic;

	void PrintText(const QString & str);
	void PrintSystem(const QString & str);
	void PrintError(const QString & str);
	void SendChannelText(const QString & message);
	void CheckScrollState();
	void UpdateTextView();
	void Action(const QString & name, const QString & msg, bool batch = false);

	void UpdateNode();

	WUserRef FindUser(const QString & user);



public slots:
	void TabPressed(const QString &);
	void URLClicked(const QString &);
	void BeforeShown();
	void GotShown(const QString &);
	void UpdateUserList();
	void UpdateTopic();
	void NewChannelText(const QString &, const QString &, const QString &);
	void ChannelAdminsChanged(const QString &, const QString &);
	void UserDisconnected(const QString &, const QString &);

};

#endif // CHANNELIMPL_H
