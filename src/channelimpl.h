#ifndef CHANNELIMPL_H
#define CHANNELIMPL_H

#include <qapplication.h>
#include <qcoreevent.h>
#include <qdialog.h>
#include <qevent.h>
#include <qsplitter.h>
#include <q3hgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <q3listview.h>

#include "chattext.h"
#include "chatwindow.h"
#include "htmlview.h"
#include "user.h"
#include "Log.h"

class NetClient;

class Channel : public ChatWindow
{
    Q_OBJECT

public:
	Channel( QWidget* parent = 0, NetClient * net = 0, QString cname = QString::null, const char* name = 0, Qt::WFlags fl = 0);
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
	virtual void customEvent(QEvent *);
	virtual void resizeEvent(QResizeEvent *);

	friend class Channels;
	void StartLogging();
	void StopLogging();

private:
	NetClient * fNet;
	QString fTopic, fOwner, fName, fStrAdmins;
	QSplitter * fSplit;
	QSplitter * fSplitBottom;
	QSplitter * fSplitChat;
	Q3HGroupBox * fTopicBox;
	QLabel * fTopicLabel;
	QLineEdit * fTopicEdit;
	Q3ListView * fChannelUsers;
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
