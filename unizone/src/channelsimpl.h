#ifndef CHANNELS_H
#define CHANNELS_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "channels.h"
#include "channelimpl.h"
#include "channelinfo.h"
#include "netclient.h"

#include <qlistview.h>

#include <map>
using std::pair;
using std::multimap;
using std::iterator;


typedef pair<QString, ChannelInfo *> WChannelPair;
typedef multimap<QString, ChannelInfo *> WChannelMap;
typedef WChannelMap::iterator WChannelIter;

class Channels : public ChannelsBase
{ 
    Q_OBJECT

public:
    Channels( QWidget* parent = 0, NetClient * net = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~Channels();

	bool IsOwner(QString channel, QString user);
	bool IsPublic(QString channel);
	void SetPublic(QString channel, bool pub);

	void PartChannel(QString channel, QString user = QString::null);

	bool IsAdmin(QString channel, QString user);
	void AddAdmin(QString channel, QString user);
	void RemoveAdmin(QString channel, QString user);
	QString GetAdmins(QString channel);

	void SetTopic(QString channel, QString topic);

public slots:
	void ChannelAdded(const QString, const QString, int64);
	void ChannelTopic(const QString, const QString, const QString);
	void ChannelAdmins(const QString, const QString, const QString);
	void CreateChannel();
	void JoinChannel();
	void ChannelCreated(const QString, const QString, int64);
	void ChannelJoin(const QString, const QString);
	void ChannelPart(const QString, const QString);
	void ChannelInvite(const QString, const QString, const QString);
	void ChannelKick(const QString, const QString, const QString);
	void ChannelPublic(const QString, const QString, bool);
	void ChannelOwner(const QString, const QString, const QString);
	void UserIDChanged(QString, QString);

signals:
	void ChannelAdminsChanged(const QString, const QString);
	void Closed();
private:
	WChannelMap fChannels;
	NetClient * fNet;

	void UpdateAdmins(WChannelIter iter);
	void UpdateUsers(WChannelIter iter);
	void UpdateTopic(WChannelIter iter);
	void UpdatePublic(WChannelIter iter);
	void JoinChannel(QString channel);
};

#endif // CHANNELSIMPL_H