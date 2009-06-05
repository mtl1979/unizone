#include "downloadqueue.h"
#include "global.h"
#include "winsharewindow.h"
#include "settings.h"
#include "downloadimpl.h"
#include "messageutil.h"

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
DownloadQueue::addItem(const QString & file, const QString & path, const WUserRef & user)
{
	int32 i = 0;
	String mUser = (const char *) user()->GetUserID().utf8();
	fLock.Lock();
        if (fQueue()->FindInt32(mUser, i) == B_OK)
		fQueue()->ReplaceInt32(false, mUser, ++i);
	else
		fQueue()->AddInt32(mUser, 1);
	mUser = mUser.Prepend("_");
	AddStringToMessage(fQueue, mUser, file);
	mUser = mUser.Prepend("path");
	AddStringToMessage(fQueue, mUser, path);
	fLock.Unlock();
}

void
DownloadQueue::run()
{
	String mUser;
	QString user;
	QString * files;
	QString * paths;
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
			int32 i;

                        fQueue()->FindInt32(mUser, numItems);
			files = new QString[numItems];
			Q_CHECK_PTR(files);
			paths = new QString[numItems];
			Q_CHECK_PTR(paths);
			mUser = mUser.Prepend("_");
			for (i = 0; i < numItems; i++)
			{
				GetStringFromMessage(fQueue, mUser, i, files[i]);
			}
			mUser = mUser.Prepend("path");
			for (i = 0; i < numItems; i++)
			{
				GetStringFromMessage(fQueue, mUser, i, paths[i]);
			}
			gWin->OpenDownload();
			if (u()->GetFirewalled() && u()->GetTunneling() && gWin->fSettings->GetFirewalled())
				gWin->fDLWindow->CreateTunnel(files, NULL, paths, numItems, u);
			else
				gWin->fDLWindow->AddDownload(files, NULL, paths, numItems, u()->GetUserID(), u()->GetPort(), u()->GetUserHostName(), u()->GetInstallID(), u()->GetFirewalled(), u()->GetPartial());
		}
	}
	fQueue()->Clear();
	fLock.Unlock();
}

