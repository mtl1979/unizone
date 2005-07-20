#ifndef CHANNELS_H
#define CHANNELS_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <qdialog.h>
#include <qlayout.h>
#include <qlistview.h>
#include <channelinfo.h>

#include <map>
using std::map;
using std::multimap;
using std::pair;

#include "support/MuscleSupport.h"

using namespace muscle;

class NetClient;

typedef pair<QString, ChannelInfo *> WChannelPair;
typedef multimap<QString, ChannelInfo *> WChannelMap;
typedef WChannelMap::iterator WChannelIter;

class Channels : public QDialog
{
	Q_OBJECT
public:
	Channels(QWidget * parent, NetClient * fNet);
	virtual ~Channels();

	bool IsOwner(const QString & channel, const QString & user);
	bool IsPublic(const QString & channel);
	void SetPublic(const QString & channel, bool pub);

	void PartChannel(const QString & channel, const QString & user);

	bool IsAdmin(const QString & channel, const QString & user);
	void AddAdmin(const QString & channel, const QString & user);
	void RemoveAdmin(const QString & channel, const QString & user);
	QString GetAdmins(const QString & channel);

	void SetTopic(const QString & channel, const QString & topic);

protected:
	friend class WinShareWindow;

	void ChannelCreated(const QString &, const QString &, uint64);
	void ChannelJoin(const QString &, const QString &);
	void ChannelPart(const QString &, const QString &);
	void ChannelInvite(const QString &, const QString &, const QString &);
	void ChannelKick(const QString &, const QString &, const QString &);

private:
	NetClient * fNetClient;

//	QWidget * fChannelsWidget;
	QGridLayout * fChannelsTab;
	QListView * ChannelList;
	QPushButton * Create;
	QPushButton * Join;

	// Channels


	WChannelMap fChannels;

	void UpdateAdmins(WChannelIter iter);
	void UpdateUsers(WChannelIter iter);
	void UpdateTopic(WChannelIter iter);
	void UpdatePublic(WChannelIter iter);
	void JoinChannel(const QString &channel);

public slots:
	void ChannelAdded(const QString &, const QString &, int64);
	void ChannelAdmins(const QString &, const QString &, const QString &);
	void ChannelTopic(const QString &, const QString &, const QString &);
	void ChannelPublic(const QString &, const QString &, bool);
	void ChannelOwner(const QString &, const QString &, const QString &);
	void UserIDChanged(const QString &, const QString &);

private slots:
	void CreateChannel();
	void JoinChannel();


signals:
	void ChannelAdminsChanged(const QString &, const QString &);

};

#endif