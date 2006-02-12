#include "downloadqueue.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "downloadimpl.h"
#include "util.h"

DownloadQueue::DownloadQueue()
{
	fLock.Lock();
	fQueue = GetMessageFromPool();
	fLock.Unlock();
}

DownloadQueue::~DownloadQueue()
{
}

void
DownloadQueue::addItem(const QString & file, const WUserRef & user)
{
	int32 i = 0;
	String mUser = (const char *) user()->GetUserID().utf8();
	fLock.Lock();
	if (fQueue()->FindInt32(mUser, &i) == B_OK)
		fQueue()->RemoveName(mUser);
	fQueue()->AddInt32(mUser, ++i);
	mUser = mUser.Prepend("_");
	fQueue()->AddString(mUser, (const char *) file.utf8());
	fLock.Unlock();
}

void
DownloadQueue::run()
{
	String mUser;
	QString user;
	QString * files;
	WUserRef u;
	int32 numItems;
	fLock.Lock();
	MessageFieldNameIterator iter = fQueue()->GetFieldNameIterator(B_INT32_TYPE);
	while (iter.GetNextFieldName(mUser) == B_OK)
	{
		user = QString::fromUtf8(mUser.Cstr());
		u = gWin->FindUser(user);
		if (u() != NULL)
		{
			fQueue()->FindInt32(mUser, &numItems);
			files = new QString[numItems];
			CHECK_PTR(files);
			mUser = mUser.Prepend("_");
			for (int32 i = 0; i < numItems; i++)
			{
				GetStringFromMessage(fQueue, mUser, i, files[i]);
			}
			gWin->OpenDownload();
			if (u()->GetFirewalled() && u()->GetTunneling() && gWin->fSettings->GetFirewalled())
				gWin->fDLWindow->CreateTunnel(files, NULL, numItems, u);
			else
				gWin->fDLWindow->AddDownload(files, NULL, numItems, u()->GetUserID(), u()->GetPort(), u()->GetUserHostName(), u()->GetInstallID(), u()->GetFirewalled(), u()->GetPartial());
		}
	}
	fQueue()->Clear();
	fLock.Unlock();
}

