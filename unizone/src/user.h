#ifndef USER_H
#define USER_H

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include <map>
using std::map;
using std::pair;
using std::iterator;

#include "message/Message.h"
#include "util/RefCount.h"
#include "util/Queue.h"
using namespace muscle;

#include <qstring.h>

class QListView;
class QListViewItem;

/** A work in progress class
  *	As the user message becomes more revealed, add more
  * methods to it.
  */
class WUser : public RefCountable
{
public:
	WUser(const QString & sid);
	~WUser();

	// <postmaster@raasu.org> 20021001
	void SetUserName(const QString & name);
	void SetStatus(const QString & s); 
	void SetUserHostName(const QString & h);
	
	void SetCurUploads(uint32 c);
	void SetMaxUploads(uint32 m);
	void SetBandwidthLabel(const char * s);
	void SetBandwidthBPS(uint32 bps);
	void SetFileCount(int32 fc);
	void SetFirewalled(bool f);
	void SetClient(const char * c);
	void SetClient(const String & s);
	void SetClient(const QString & s);
	void SetPartial(bool p) { fPartial = p; }

	void UpdateListViews();

	QString GetUserName() const { return fUserName; }
	QString GetStatus() const { return fUserStatus; }
	QString GetUserID() const { return fUserID; }
	QString GetUserHostName() const { return fHostName; }
	QString GetHostOS() const { return fHostOS; }

	uint32 GetCurUploads() const { return fCurUploads; }
	uint32 GetMaxUploads() const { return fMaxUploads; }
	QString GetBandwidthLabel() const { return fBandwidthLabel; }
	uint32 GetBandwidthBPS() const { return fBandwidthBPS; }
	int32 GetFileCount() const { return fFileCount; }
	bool GetFirewalled() const { return fFirewalled; }
	bool GetPartial() const { return fPartial; }
	bool GetTunneling() const { return fTunneling; }

	QString GetClient() const { return fClient; }

	// check state only...
	bool IsBot() const { return fBot; }
	uint32 GetPort() const { return fPort; }
	uint64 GetInstallID() const { return fInstallID; }

	void InitName(MessageRef msg);
	void InitStatus(MessageRef msg);
	void InitUploadStats(MessageRef msg);
	void InitBandwidth(MessageRef msg);
	void InitFileCount(MessageRef msg);
	
	void AddToListView(QListView * view);
	void RemoveFromListView(QListView * view = NULL);	// if NULL, remove from all list views

	bool NeedPing() const { return fNeedPing; }

	QString GetLastLine() const { return fLastLine; }
	void SetLastLine(const QString & line) { fLastLine = line; }

	QString GetLastLine(const QString & channel) const;
	void SetLastLine(const QString & channel, const QString & line);

private:
	QString fHostName;
	QString fHostOS;
	QString fUserID;
	QString fUserStatus;
	QString fUserName;

	uint32 fCurUploads, fMaxUploads;
	QString fBandwidthLabel;
	uint32 fBandwidthBPS;
	int32 fFileCount;
	bool fFirewalled;
	bool fPartial;
	bool fTunneling;

	QString fClient;

	bool fBot;
	uint32 fPort;
	uint64 fInstallID;

	bool fNeedPing;

	QString fLastLine;

	//

	typedef map<QListView *, QListViewItem *> WListMap;	// holds a list of QListView/QListViewItem pairs
	typedef WListMap::iterator WListIter;
	typedef pair<QListView *, QListViewItem *> WListPair;
	typedef pair<QString, QString> QStringPair;

	Queue<QStringPair> fLastLines;						// holds last lines for channels

	WListMap fLists;

	// private inline method for generating a pair<>
	WListPair 
	MakeListPair(QListView * v, QListViewItem * i)
	{
		WListPair p;
		p.first = v;
		p.second = i;
		return p;
	}
};

typedef Ref<WUser> WUserRef;


typedef Hashtable<uint32, WUserRef> WUserMap;
typedef HashtableIterator<uint32, WUserRef> WUserIter;

typedef pair<WUserRef, QString> WUserSearchPair;
typedef Queue<WUserSearchPair> WUserSearchMap;

inline WUserSearchPair
MakePair(const WUserRef & w, const QString &txt)
{
	WUserSearchPair p;
	p.first = w;
	p.second = txt;
	return p;
}


#endif

