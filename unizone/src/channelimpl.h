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
#include "netclient.h"
#include "chattext.h"
#include "htmlview.h"

class Channel : public ChannelBase
{ 
    Q_OBJECT

public:
    Channel( QWidget* parent = 0, NetClient * net = 0, QString cname = QString::null, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~Channel();
	void SetOwner(QString);
	void SetTopic(QString);
	void SetActive(bool);
	void SetPublic(bool);
	void AddUser(QString user);
	bool RemUser(QString user);
	void Invite(QString user);
	void Kick(QString user);
protected:
	virtual void customEvent(QCustomEvent *);
	virtual void resizeEvent(QResizeEvent * e);

private:
	NetClient * fNet;
	QString fTopic, fOwner, fName, fURL, fStrAdmins;
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
	void SendChannelText(QString message);
	void CheckScrollState();
	void UpdateView();
	void Action(const QString & name, const QString & msg, bool batch = false);

	void UpdateNode();


public slots:
	void TabPressed(QString str);
	void URLClicked();
	void GotShown(const QString & txt);
	void URLSelected(const QString & href);
	void UpdateUserList();
	void UpdateTopic();
	void NewChannelText(const QString, const QString, const QString);
	void ChannelAdminsChanged(const QString, const QString);
	void UserDisconnected(QString sid, QString name);

};

#endif // CHANNELIMPL_H
