#include "channels.h"
#include "winsharewindow.h"
#include "channelinfo.h"
#include "channelimpl.h"
#include "tokenizer.h"
#include "debugimpl.h"
#include "global.h"
#include "wstring.h"
#include "netclient.h"

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

Channels::Channels(QWidget *parent, NetClient *fNet)
:QDialog(parent, "Channels", false, WStyle_SysMenu | WStyle_Minimize | WStyle_Maximize | WStyle_Title)
{
	setCaption(tr("Channels"));

	fNetClient = fNet;

	// Create the Channels Pane

//	fChannelsWidget = new QWidget(this, "Channels Widget");
//	CHECK_PTR(fChannelsWidget);

	QGridLayout * fChannelsTab = new QGridLayout(this, 7, 5, 0, -1, "Channels Tab");
	CHECK_PTR(fChannelsTab);

	fChannelsTab->addRowSpacing(5, 20);

	fChannelsTab->setRowStretch(0, 2);
	fChannelsTab->setRowStretch(1, 2);
	fChannelsTab->setRowStretch(2, 2);
	fChannelsTab->setRowStretch(3, 2);
	fChannelsTab->setRowStretch(4, 2);
	fChannelsTab->setRowStretch(5, 0);
	fChannelsTab->setRowStretch(6, 1);

	fChannelsTab->addColSpacing(0, 20);
	fChannelsTab->addColSpacing(2, 20);
	fChannelsTab->addColSpacing(4, 20);

	ChannelList = new QListView( this, "ChannelList" );
	CHECK_PTR(ChannelList);

	ChannelList->addColumn( tr( "Name" ) );
	ChannelList->addColumn( tr( "Topic" ) );
	ChannelList->addColumn( tr( "Users" ) );
	ChannelList->addColumn( tr( "Admins" ) );
	ChannelList->addColumn( tr( "Public" ) );

	fChannelsTab->addMultiCellWidget(ChannelList, 0, 4, 0, 4);
	
	Create = new QPushButton( tr( "&Create" ), this, "Create" );
	CHECK_PTR(Create);

	fChannelsTab->addWidget(Create, 6, 1);

	Join = new QPushButton( tr( "&Join" ), this, "Join" );
	CHECK_PTR(Join);

	fChannelsTab->addWidget(Join, 6, 3);

	connect( 
		fNetClient, SIGNAL(ChannelAdded(const QString &, const QString &, int64)), 
		this, SLOT(ChannelAdded(const QString &, const QString &, int64)) 
		);
	connect(
		fNetClient, SIGNAL(ChannelTopic(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelTopic(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(ChannelOwner(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelOwner(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(ChannelPublic(const QString &, const QString &, bool)),
		this, SLOT(ChannelPublic(const QString &, const QString &, bool))
		);
	connect(
		fNetClient, SIGNAL(ChannelAdmins(const QString &, const QString &, const QString &)),
		this, SLOT(ChannelAdmins(const QString &, const QString &, const QString &))
		);
	connect(
		fNetClient, SIGNAL(UserIDChanged(const QString &, const QString &)),
		this, SLOT(UserIDChanged(const QString &, const QString &))
		);

	// Window widget events
	connect(Create, SIGNAL(clicked()), this, SLOT(CreateChannel()));
	connect(Join, SIGNAL(clicked()), this, SLOT(JoinChannel()));

	//
	// End of Channels Pane
	//
}

Channels::~Channels()
{
}

void
Channels::ChannelAdmins(const QString &channel, const QString &sid, const QString &admins)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		if ( (*iter).second->IsAdmin(sid) )
		{
			if ( (*iter).second->SetAdmins(admins) )
				UpdateAdmins(iter);
		}
	}
}

bool
Channels::IsAdmin(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->IsAdmin(user);
	}
	return false;
}

bool
Channels::IsOwner(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->IsOwner(user);
	}
	return false;
}

bool
Channels::IsPublic(const QString & channel)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->GetPublic();
	}
	return false;
}

void
Channels::ChannelAdded(const QString &channel, const QString &sid, int64 timecreated)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter == fChannels.end())
	{
		WChannelPair wcp;
		wcp.first = channel;
		wcp.second = new ChannelInfo(channel, sid);
		fChannels.insert(fChannels.end(), wcp);
		iter = fChannels.find(channel);
		// Create ListView Item
		QListViewItem * item = new QListViewItem(ChannelList, channel, "", "", "", "");
		wcp.second->SetItem(item);
		wcp.second->SetCreated(timecreated);
	}
	(*iter).second->AddUser(sid);
	// Send user list to window
	QStringTokenizer tok((*iter).second->GetUsers(), ",");
	QString next;
	while ((next = tok.GetNextToken()) != QString::null)
	{
		if ( (*iter).second->GetWindow() )
		{
			(*iter).second->GetWindow()->AddUser(next);
		}
	}
	UpdateAdmins(iter);
	UpdateTopic(iter);
	UpdateUsers(iter);
	UpdatePublic(iter);
}

void
Channels::UpdateAdmins(WChannelIter iter)
{
	QListViewItem *item = (*iter).second->GetItem();
	if (item)
		item->setText(3, QString::number( (*iter).second->NumAdmins() ));
	emit ChannelAdminsChanged((*iter).first, (*iter).second->GetAdmins());
}

void
Channels::UpdateUsers(WChannelIter iter)
{
	QListViewItem *item = (*iter).second->GetItem();
	int n = (*iter).second->NumUsers();
	if (item)
		item->setText(2, QString::number( n ));
}

void
Channels::UpdateTopic(WChannelIter iter)
{
	QListViewItem *item = (*iter).second->GetItem();
	if (item)
		item->setText(1, (*iter).second->GetTopic() );
}

void
Channels::UpdatePublic(WChannelIter iter)
{
	QListViewItem *item = (*iter).second->GetItem();
	if (item)
		item->setText(4, (*iter).second->GetPublic() ? tr( "Yes" ) : tr( "No" ) );
}

void
Channels::CreateChannel()
{
	if (!fNetClient->IsConnected())
	{
		// Don't create, when we aren't connected
		return;
	}

	bool ok = FALSE;
	QString text = QInputDialog::getText( 
		tr( "Create Channel" ), 
		tr( "Please enter channel name" ),
#if (QT_VERSION >= 0x030000)
		QLineEdit::Normal, 
#endif
		QString::null, &ok, this 
		);
	if ( ok && !text.isEmpty() )
	{
		// user entered something and pressed ok
		WChannelIter it = fChannels.find(text);
		if (it == fChannels.end())
		{
			// Create Channel
			WChannelPair wcp;
			wcp.first = text;
			wcp.second = new ChannelInfo(text, gWin->GetUserID());
			fChannels.insert(fChannels.end(), wcp);

			it = fChannels.find(text);
			// Create ListView item
			QListViewItem * item = new QListViewItem(ChannelList, text, "", "", "", "");
			wcp.second->SetItem(item);
			// Create Window item
			Channel * win = new Channel(this, fNetClient, text);
			wcp.second->SetWindow(win);
			wcp.second->AddUser(gWin->GetUserID());
			win->SetOwner(gWin->GetUserID());
			win->show();

			ChannelAdmins(text, gWin->GetUserID(), gWin->GetUserID());
			UpdateUsers(it);
			UpdatePublic(it);

			// Send Channel Data message
			MessageRef cc(GetMessageFromPool(NetClient::ChannelData));
			if (cc())
			{
				String to("/*/*/unishare");
				cc()->AddString(PR_NAME_KEYS, to);
				cc()->AddString(PR_NAME_SESSION, (const char *) gWin->GetUserID().utf8());
				cc()->AddInt64("when", (int64) GetCurrentTime64());
				cc()->AddString("channel", (const char *) text.utf8());
				fNetClient->SendMessageToSessions(cc);
			}
		}
	}
	else
	{
		// user entered nothing or pressed cancel
	}
}

void
Channels::JoinChannel()
{
	QListViewItem *item = ChannelList->selectedItem();
	if (!item)
		return;
	QString text = item->text(0); // Get Channel name
	JoinChannel(text);
}

void
Channels::JoinChannel(const QString & channel)
{
	WChannelIter it = fChannels.find(channel);
	Channel * win;
	if (!(*it).second->GetWindow())
	{
		//
		// Create Channel Window
		//
		
		// Create Window item
		win = new Channel(this, fNetClient, channel);
		(*it).second->SetWindow(win);
	}
	else
	{
		win = (*it).second->GetWindow();
	}
	
	// Send user list to window
#ifdef _DEBUG
	WString wUsers((*it).second->GetUsers());
	PRINT("JoinChannel: GetUsers = %S\n", wUsers.getBuffer());
#endif
	int n = fNetClient->GetUserCount(channel);
	if (n > 0)
	{
		int i = 0;
		QString * user = fNetClient->GetChannelUsers(channel);
		while (i < n)
		{
			(*it).second->GetWindow()->AddUser(user[i++]);
		}
		delete [] user;
	}
	
	(*it).second->AddUser(gWin->GetUserID());
	win->SetOwner((*it).second->GetOwner());
	win->SetTopic((*it).second->GetTopic());
	win->SetPublic((*it).second->GetPublic());
	win->show();
	
	UpdateUsers(it);
	UpdatePublic(it);
	
	if (!(*it).second->GetPublic()) // Needs invite?
	{
		// Send Channel Invite message
		MessageRef cc(GetMessageFromPool(NetClient::ChannelInvite));
		if (cc())
		{
			String to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, to);
			cc()->AddString(PR_NAME_SESSION, (const char *) gWin->GetUserID().utf8());
			cc()->AddString("who", (const char *) gWin->GetUserID().utf8());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNetClient->SendMessageToSessions(cc);
		}
	}	
}

void
Channels::ChannelCreated(const QString & channel, const QString & owner, uint64 timecreated)
{
	WChannelIter it = fChannels.find(channel);
	if (it == fChannels.end())
	{
		// Create Channel
		WChannelPair wcp;
		wcp.first = channel;
		wcp.second = new ChannelInfo(channel, gWin->GetUserID());
		fChannels.insert(fChannels.end(), wcp);

		it = fChannels.find(channel);
		// Create ListView item
		QListViewItem * item = new QListViewItem(ChannelList, channel, "", "", "", "");
		wcp.second->SetItem(item);
		ChannelAdmins(owner, channel, owner);
		ChannelJoin(channel, owner);
		UpdatePublic(it);
		UpdateUsers(it);
	}
	else if (timecreated >= (*it).second->GetCreated())
	{
		// Send Channel Data message
		MessageRef cc(GetMessageFromPool(NetClient::ChannelKick));
		if (cc())
		{
			QString to("/*/");
			to += owner;
			to += "/unishare";
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString(PR_NAME_SESSION, (const char *) gWin->GetUserID().utf8());
			cc()->AddInt64("when", (int64) (*it).second->GetCreated());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNetClient->SendMessageToSessions(cc);
		}
	}
	else
	{
		(*it).second->SetCreated(timecreated);
		(*it).second->SetOwner(owner);
	}
}

void
Channels::ChannelJoin(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->AddUser(user);
		UpdateUsers(iter);
	}
}

void
Channels::ChannelPart(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->RemoveUser(user);
		UpdateUsers(iter);
	}
}

void
Channels::PartChannel(const QString & channel, const QString & user)
{
	if (user == gWin->GetUserID())
	{
		MessageRef cc(GetMessageFromPool(NetClient::ChannelPart));
		if (cc())
		{
			String to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, to);
			cc()->AddString(PR_NAME_SESSION, (const char *) gWin->GetUserID().utf8());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNetClient->SendMessageToSessions(cc);
		}
	}

	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		// Make sure we don't have a stale window reference, so we can re-join later
		if (user == gWin->GetUserID())
		{
			(*iter).second->SetWindow(NULL);
		}
		(*iter).second->RemoveUser(user);
		UpdateUsers(iter);
		if ((*iter).second->NumUsers() == 0)
		{
			delete (*iter).second;
			fChannels.erase(iter);
		}
	}
}

void
Channels::AddAdmin(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		WUserRef uref = gWin->FindUser(user);
		if (uref())
		{
			(*iter).second->AddAdmin(uref()->GetUserID());
			UpdateAdmins(iter);
		}
	}
}

void
Channels::RemoveAdmin(const QString & channel, const QString & user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		WUserRef uref = gWin->FindUser(user);
		if (uref())
		{
			(*iter).second->RemoveAdmin(uref()->GetUserID());
			UpdateAdmins(iter);
		}
	}
}

QString
Channels::GetAdmins(const QString & channel)
{
	QString adm = QString::null;
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		adm = (*iter).second->GetAdmins();
	}
	return adm;
}

void
Channels::SetTopic(const QString & channel, const QString & topic)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->SetTopic(topic);
		UpdateTopic(iter);
	}
}

void
Channels::SetPublic(const QString & channel, bool pub)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->SetPublic(pub);
		UpdatePublic(iter);
	}
}

void
Channels::ChannelInvite(const QString & channel, const QString & user, const QString & who)
{
	WChannelIter iter = fChannels.find(channel);
	// We need to have existing channel to be able to check for admin status
	if (
		(who == gWin->GetUserID()) && 
		(iter == fChannels.end())
		)
	{
		ChannelAdded(channel, user, GetCurrentTime64());
		iter = fChannels.find(channel);
	}

	if (IsAdmin(channel, user))
	{
		if (who == gWin->GetUserID())
		{
			// Got invited
			if (!(*iter).second->GetWindow())
			{
				if (QMessageBox::information(this, tr( "Channels" ), 
					tr( "User #%1 invited you to channel %2." " Do you accept?").arg(user).arg(channel),
					tr( "Yes" ), tr( "No" )) == 0)	// 0 is the index of "yes"
				{
					Channel * win = new Channel(this, fNetClient, channel);
					win->SetOwner((*iter).second->GetOwner());
					(*iter).second->SetWindow(win);
					win->show();
				}
				else
				{
					return;
				}
			}
			(*iter).second->GetWindow()->SetActive(true);
		}
		else if ( IsAdmin(channel, gWin->GetUserID()) && (*iter).second->GetWindow() )
		{
			// User requested invite from us
			if (QMessageBox::information(this, tr( "Channels" ), 
				tr( "User #%1 requested invite to channel %2." " Do you?").arg(user).arg(channel),
				tr( "Yes" ), tr( "No" )) == 0)	// 0 is the index of "yes"
			{
				(*iter).second->GetWindow()->Invite(user);
			}
		}
	}
}

void
Channels::ChannelKick(const QString &channel, const QString &user, const QString &who)
{
	WChannelIter iter = fChannels.find(channel);
	if (IsAdmin(channel, user))
	{
		if (who == gWin->GetUserID())
		{
			// Got kicked
			if ( (iter != fChannels.end()) && (*iter).second->GetWindow() )
			{
				(*iter).second->GetWindow()->SetActive(false);
			}
		}
	}
}

void
Channels::ChannelTopic(const QString &channel, const QString &user, const QString &topic)
{
	WChannelIter iter = fChannels.find(channel);
	if (IsAdmin(channel, user))
	{
		// Topic changed
		if (iter != fChannels.end())
		{
			if ( (*iter).second->GetWindow() )
			{
				(*iter).second->GetWindow()->SetTopic(topic);
			}
			else
			{
				(*iter).second->SetTopic(topic);
				UpdateTopic(iter);
			}
		}
	}
}

void
Channels::ChannelOwner(const QString &channel, const QString &user, const QString &owner)
{
	WChannelIter iter = fChannels.find(channel);
	if (IsAdmin(channel, user))
	{
		// Owner changed
		if (iter != fChannels.end())
		{
			if ( (*iter).second->GetWindow() )
			{
				(*iter).second->GetWindow()->SetOwner(owner);
			}
			else
			{
				(*iter).second->SetOwner(owner);
			}
		}
	}
}

void
Channels::ChannelPublic(const QString &channel, const QString &user, bool pub)
{
	WChannelIter iter = fChannels.find(channel);
	if (IsAdmin(channel, user))
	{
		// Public/Private changed
		if (iter != fChannels.end())
		{
			if ( (*iter).second->GetWindow() )
			{
				(*iter).second->GetWindow()->SetPublic(pub);
			}
		}
	}
}

void
Channels::UserIDChanged(const QString &oldid, const QString &newid)
{
	for (WChannelIter iter = fChannels.begin(); iter != fChannels.end(); iter++)
	{
		if (IsOwner((*iter).first, oldid))
		{
			// Owner changed
			(*iter).second->SetOwner(newid);
			if ((*iter).second->GetWindow())
			{
				(*iter).second->GetWindow()->SetOwner(newid);
			}
		}
		if (IsAdmin((*iter).first, oldid))
		{
			// Admin re-entered
			(*iter).second->RemoveAdmin(oldid);
			(*iter).second->AddAdmin(newid);
		}
		(*iter).second->RemoveUser(oldid);
		(*iter).second->AddUser(newid);
#ifdef _DEBUG
		WString wMyID(gWin->GetUserID());
		WString wOldID(oldid);
		WString wNewID(newid);
		PRINT("UserIDChanged, myid = %S, old = %S, new = %S\n", wMyID.getBuffer(), wOldID.getBuffer(), wNewID.getBuffer());
#endif

		if (newid == gWin->GetUserID())
		{
			// Our ID got changed
			if ((*iter).second->GetWindow())
			{
				// We need window
				JoinChannel((*iter).first);
			}
		}
	}
}
