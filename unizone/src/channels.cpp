#include <qapplication.h>
#include <Q3GridLayout>

#include "channels.h"
#include "winsharewindow.h"
#include "channelinfo.h"
#include "channelimpl.h"
#include "tokenizer.h"
#include "debugimpl.h"
#include "global.h"
#include "wstring.h"
#include "netclient.h"
#include "messageutil.h"

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

Channels::Channels(QWidget *parent, NetClient *fNet)
:QDialog(parent, "Channels", false, Qt::WStyle_SysMenu | Qt::WStyle_Minimize | Qt::WStyle_Maximize | Qt::WStyle_Title)
{
	setCaption(tr("Channels"));

	fNetClient = fNet;

	// Create the Channels Pane

	Q3GridLayout * fChannelsTab = new Q3GridLayout(this, 7, 5, 0, -1, "Channels Tab");
	Q_CHECK_PTR(fChannelsTab);

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

	ChannelList = new Q3ListView( this, "ChannelList" );
	Q_CHECK_PTR(ChannelList);
	connect(ChannelList, SIGNAL(SelectionChanged()), this, SLOT(ChannelSelected()));

	ChannelList->addColumn( tr( "Name" ) );
	ChannelList->addColumn( tr( "Topic" ) );
	ChannelList->addColumn( tr( "Users" ) );
	ChannelList->addColumn( tr( "Admins" ) );
	ChannelList->addColumn( tr( "Public" ) );

	fChannelsTab->addMultiCellWidget(ChannelList, 0, 4, 0, 4);
	
	Create = new QPushButton( tr( "&Create" ), this, "Create" );
	Q_CHECK_PTR(Create);
	Create->setEnabled(fNetClient->IsConnected());

	fChannelsTab->addWidget(Create, 6, 1);

	Join = new QPushButton( tr( "&Join" ), this, "Join" );
	Q_CHECK_PTR(Join);
	Join->setEnabled(false);


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
	connect(fNetClient, SIGNAL(ConnectedToServer()), this, SLOT(ServerConnected()));
	connect(fNetClient, SIGNAL(DisconnectedFromServer()), this, SLOT(ServerDisconnected()));
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
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if ( info->IsAdmin(sid) )
		{
			if ( info->SetAdmins(admins) )
				UpdateAdmins(channel, info);
		}
	}
}

bool
Channels::IsAdmin(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		return info->IsAdmin(user);
	}
	return false;
}

bool
Channels::IsOwner(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		return info->IsOwner(user);
	}
	return false;
}

bool
Channels::IsPublic(const QString & channel)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		return info->GetPublic();
	}
	return false;
}

void
Channels::ChannelAdded(const QString &channel, const QString &sid, int64 timecreated)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) != B_OK)
	{
		info = new ChannelInfo(channel, sid);
		fChannels.Put(channel, info);
		// Create ListView Item
		Q3ListViewItem * item = new Q3ListViewItem(ChannelList, channel, QString::null, QString::null, QString::null, QString::null);
		info->SetItem(item);
		info->SetCreated(timecreated);
	}
	info->AddUser(sid);
	// Send user list to window
	if ( info->GetWindow() )
	{
		QStringTokenizer tok(info->GetUsers(), ",");
		QString next;
		while ((next = tok.GetNextToken()) != QString::null)
		{
			info->GetWindow()->AddUser(next);
		}
	}
	UpdateAdmins(channel, info);
	UpdateTopic(info);
	UpdateUsers(info);
	UpdatePublic(info);
}

void
Channels::UpdateAdmins(const QString &channel, ChannelInfo * info)
{
	Q3ListViewItem *item = info->GetItem();
	if (item)
		item->setText(3, QString::number( info->NumAdmins() ));
	emit ChannelAdminsChanged(channel, info->GetAdmins());
}

void
Channels::UpdateUsers(ChannelInfo * info)
{
	Q3ListViewItem *item = info->GetItem();
	if (item)
	{
		int n = info->NumUsers();
		item->setText(2, QString::number( n ));
	}
}

void
Channels::UpdateTopic(ChannelInfo * info)
{
	Q3ListViewItem *item = info->GetItem();
	if (item)
		item->setText(1, info->GetTopic() );
}

void
Channels::UpdatePublic(ChannelInfo * info)
{
	Q3ListViewItem *item = info->GetItem();
	if (item)
		item->setText(4, info->GetPublic() ? tr( "Yes" ) : tr( "No" ) );
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
	QString channel = QInputDialog::getText( 
		tr( "Create Channel" ), 
		tr( "Please enter channel name" ),
		QLineEdit::Normal, 
		QString::null, &ok, this 
		);
	if ( ok && !channel.isEmpty() )
	{
		// user entered something and pressed ok
		if (!fChannels.ContainsKey(channel))
		{
			// Create Channel
			ChannelInfo * info = new ChannelInfo(channel, gWin->GetUserID());
			fChannels.Put(channel, info);

			// Create ListView item
			Q3ListViewItem * item = new Q3ListViewItem(ChannelList, channel, QString::null, QString::null, QString::null, QString::null);
			info->SetItem(item);
			// Create Window item
			Channel * win = new Channel(this, fNetClient, channel);
			info->SetWindow(win);
			info->AddUser(gWin->GetUserID());
			win->SetOwner(gWin->GetUserID());
			win->show();

			ChannelAdmins(channel, gWin->GetUserID(), gWin->GetUserID());
			UpdateUsers(info);
			UpdatePublic(info);

			// Send Channel Data message
			MessageRef cc(GetMessageFromPool(NetClient::ChannelData));
			if (cc())
			{
				String to("/*/*/unishare");
				cc()->AddString(PR_NAME_KEYS, to);
				AddStringToMessage(cc, PR_NAME_SESSION, gWin->GetUserID());
				cc()->AddInt64("when", (int64) GetCurrentTime64());
				AddStringToMessage(cc, "channel", channel);
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
	Q3ListViewItem *item = ChannelList->selectedItem();
	if (item)
	{
		QString text = item->text(0); // Get Channel name
		JoinChannel(text);
	}
}

void
Channels::JoinChannel(const QString & channel)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		Channel * win;
		if (info->GetWindow())
		{
			win = info->GetWindow();
		}
		else
		{
			//
			// Create Channel Window
			//
			
			// Create Window item
			win = new Channel(this, fNetClient, channel);
			info->SetWindow(win);
		}
		
		// Send user list to window
#ifdef _DEBUG
		WString wusers(info->GetUsers());
		PRINT("JoinChannel: GetUsers = %S\n", wusers.getBuffer());
#endif
		int n = fNetClient->GetUserCount(channel);
		if (n > 0)
		{
			int i = 0;
			QString * user = fNetClient->GetChannelUsers(channel);
			while (i < n)
			{
				info->GetWindow()->AddUser(user[i++]);
			}
			delete [] user;
		}
		
		info->AddUser(gWin->GetUserID());
		win->SetOwner(info->GetOwner());
		win->SetTopic(info->GetTopic());
		win->SetPublic(info->GetPublic());
		win->show();
		
		UpdateUsers(info);
		UpdatePublic(info);
		
		if (!info->GetPublic()) // Needs invite?
		{
			// Send Channel Invite message
			MessageRef cc(GetMessageFromPool(NetClient::ChannelInvite));
			if (cc())
			{
				String to("/*/*/unishare");
				cc()->AddString(PR_NAME_KEYS, to);
				AddStringToMessage(cc, PR_NAME_SESSION, gWin->GetUserID());
				AddStringToMessage(cc, "who", gWin->GetUserID());
				cc()->AddInt64("when", (int64) GetCurrentTime64());
				AddStringToMessage(cc, "channel", channel);
				fNetClient->SendMessageToSessions(cc);
			}
		}	
	}
}

void
Channels::ChannelCreated(const QString & channel, const QString & owner, uint64 timecreated)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (timecreated >= info->GetCreated())
		{
			// Send Channel Data message
			MessageRef cc(GetMessageFromPool(NetClient::ChannelKick));
			if (cc())
			{
				QString to("/*/");
				to += owner;
				to += "/unishare";
				AddStringToMessage(cc, PR_NAME_KEYS, to);
				AddStringToMessage(cc, PR_NAME_SESSION, gWin->GetUserID());
				cc()->AddInt64("when", (int64) info->GetCreated());
				AddStringToMessage(cc, "channel", channel);
				fNetClient->SendMessageToSessions(cc);
			}
		}
		else
		{
			info->SetCreated(timecreated);
			info->SetOwner(owner);
		}	
	}
	else
	{
		// Create Channel
		ChannelInfo *info = new ChannelInfo(channel, gWin->GetUserID());
		fChannels.Put(channel, info);

		// Create ListView item
		Q3ListViewItem * item = new Q3ListViewItem(ChannelList, channel, QString::null, QString::null, QString::null, QString::null);
		info->SetItem(item);
		ChannelAdmins(owner, channel, owner);
		ChannelJoin(channel, owner);
		UpdatePublic(info);
		UpdateUsers(info);
	}
}

void
Channels::ChannelJoin(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		info->AddUser(user);
		UpdateUsers(info);
	}
}

void
Channels::ChannelPart(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		info->RemoveUser(user);
		UpdateUsers(info);
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
			AddStringToMessage(cc, PR_NAME_SESSION, gWin->GetUserID());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			AddStringToMessage(cc, "channel", channel);
			fNetClient->SendMessageToSessions(cc);
		}
	}

	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		// Make sure we don't have a stale window reference, so we can re-join later
		if (user == gWin->GetUserID())
		{
			info->SetWindow(NULL);
		}
		info->RemoveUser(user);
		UpdateUsers(info);
		if (info->NumUsers() == 0)
		{
			delete info;
			fChannels.Remove(channel);
		}
	}
}

void
Channels::AddAdmin(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		WUserRef uref = gWin->FindUser(user);
		if (uref())
		{
			info->AddAdmin(uref()->GetUserID());
			UpdateAdmins(channel, info);
		}
	}
}

void
Channels::RemoveAdmin(const QString & channel, const QString & user)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		WUserRef uref = gWin->FindUser(user);
		if (uref())
		{
			info->RemoveAdmin(uref()->GetUserID());
			UpdateAdmins(channel, info);
		}
	}
}

QString
Channels::GetAdmins(const QString & channel)
{
	QString adm = QString::null;
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		adm = info->GetAdmins();
	}
	return adm;
}

void
Channels::SetTopic(const QString & channel, const QString & topic)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		info->SetTopic(topic);
		UpdateTopic(info);
	}
}

void
Channels::SetPublic(const QString & channel, bool pub)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		info->SetPublic(pub);
		UpdatePublic(info);
	}
}

void
Channels::ChannelInvite(const QString & channel, const QString & user, const QString & who)
{
	ChannelInfo * info;
	if (!fChannels.ContainsKey(channel))
	{
		// We need to have existing channel to be able to check for admin status
		if (who == gWin->GetUserID())
		{
			ChannelAdded(channel, user, GetCurrentTime64());
		}
	}

	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (IsAdmin(channel, user))
		{
			if (who == gWin->GetUserID())
			{
				// Got invited
				if (!info->GetWindow())
				{
					if (QMessageBox::information(this, tr( "Channels" ), 
						tr( "User #%1 invited you to channel %2." " Do you accept?").arg(user).arg(channel),
						tr( "Yes" ), tr( "No" )) == 0)	// 0 is the index of "yes"
					{
						Channel * win = new Channel(this, fNetClient, channel);
						win->SetOwner(info->GetOwner());
						info->SetWindow(win);
						win->show();
					}
					else
					{
						return;
					}
				}
				info->GetWindow()->SetActive(true);
			}
			else if ( IsAdmin(channel, gWin->GetUserID()) && info->GetWindow() )
			{
				// User requested invite from us
				if (QMessageBox::information(this, tr( "Channels" ), 
					tr( "User #%1 requested invite to channel %2." " Do you?").arg(user).arg(channel),
					tr( "Yes" ), tr( "No" )) == 0)	// 0 is the index of "yes"
				{
					info->GetWindow()->Invite(user);
				}
			}
		}
	}
}

void
Channels::ChannelKick(const QString &channel, const QString &user, const QString &who)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (IsAdmin(channel, user))
		{
			if (who == gWin->GetUserID())
			{
				// Got kicked
				if ( info->GetWindow() )
				{
					info->GetWindow()->SetActive(false);
				}
			}
		}
	}
}

void
Channels::ChannelTopic(const QString &channel, const QString &user, const QString &topic)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (IsAdmin(channel, user))
		{
			// Topic changed
			if ( info->GetWindow() )
			{
				info->GetWindow()->SetTopic(topic);
			}
			else
			{
				info->SetTopic(topic);
				UpdateTopic(info);
			}
		}
	}
}

void
Channels::ChannelOwner(const QString &channel, const QString &user, const QString &owner)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (IsAdmin(channel, user))
		{
			// Owner changed
			if ( info->GetWindow() )
			{
				info->GetWindow()->SetOwner(owner);
			}
			else
			{
				info->SetOwner(owner);
			}
		}
	}
}

void
Channels::ChannelPublic(const QString &channel, const QString &user, bool pub)
{
	ChannelInfo * info;
	if (fChannels.GetValue(channel, info) == B_OK)
	{
		if (IsAdmin(channel, user))
		{
			// Public/Private changed
			if ( info->GetWindow() )
			{
				info->GetWindow()->SetPublic(pub);
			}
		}
	}
}

void
Channels::UserIDChanged(const QString &oldid, const QString &newid)
{
	WChannelIter iter = fChannels.GetIterator(HTIT_FLAG_NOREGISTER);
	ChannelInfo *info;
	QString channel;
	while (iter.HasData())
	{
		channel = iter.GetKey();
		info = iter.GetValue();
		iter++;
		if (IsOwner(channel, oldid))
		{
			// Owner changed
			info->SetOwner(newid);
			if (info->GetWindow())
			{
				info->GetWindow()->SetOwner(newid);
			}
		}
		if (IsAdmin(channel, oldid))
		{
			// Admin re-entered
			info->RemoveAdmin(oldid);
			info->AddAdmin(newid);
		}
		info->RemoveUser(oldid);
		info->AddUser(newid);
#ifdef _DEBUG
		WString wuid(gWin->GetUserID());
		WString wold(oldid);
		WString wnew(newid);
		PRINT("UserIDChanged, myid = %S, old = %S, new = %S\n", wuid.getBuffer(), wold.getBuffer(), wnew.getBuffer());
#endif

		if (newid == gWin->GetUserID())
		{
			// Our ID got changed
			if (info->GetWindow())
			{
				// We need window
				JoinChannel(channel);
			}
		}
	}
}

void
Channels::StartLogging()
{
	WChannelIter iter = fChannels.GetIterator(HTIT_FLAG_NOREGISTER);
	while (iter.HasData())
	{
		ChannelInfo * info = iter.GetValue();
		iter++;
		Channel * chn = info->GetWindow();
		if (chn)
			chn->StartLogging();
	}
}

void
Channels::StopLogging()
{
	WChannelIter iter = fChannels.GetIterator(HTIT_FLAG_NOREGISTER);
	while (iter.HasData())
	{
		ChannelInfo * info = iter.GetValue();
		iter++;
		Channel * chn = info->GetWindow();
		if (chn)
			chn->StopLogging();
	}
}

void
Channels::HandleMessage(MessageRef & msg)
{
	switch (msg()->what)
	{
		case NetClient::ChannelCreated:
			{
				QString qChan, qOwner;
				uint64 rtime;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qOwner) == B_OK) &&
					(GetStringFromMessage(msg, "channel", qChan) == B_OK) &&
										(msg()->FindInt64("when", rtime) == B_OK)
					)
				{
					ChannelCreated(qChan, qOwner, rtime);
				}

				break;
			}
		case NetClient::ChannelJoin:
			{
				QString qChan, qUser;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) && 
					(GetStringFromMessage(msg, "channel", qChan) == B_OK)
					)
				{
					ChannelJoin(qChan, qUser);
				}
				break;
			}
		case NetClient::ChannelPart:
			{
				QString qChan, qUser;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) && 
					(GetStringFromMessage(msg, "channel", qChan) == B_OK)
					)
				{
					ChannelPart(qChan, qUser);
				}
				break;
			}
		case NetClient::ChannelInvite:
			{
				QString qChan, qUser, qWho;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) &&
					(GetStringFromMessage(msg, "who", qWho) == B_OK) &&
					(GetStringFromMessage(msg, "channel", qChan) == B_OK)
					)
				{
					ChannelInvite(qChan, qUser, qWho);
				}
				break;
			}
		case NetClient::ChannelKick:
			{
				QString qChan, qUser, qWho;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) &&
					(GetStringFromMessage(msg, "who", qWho) == B_OK) &&
					(GetStringFromMessage(msg, "channel", qChan) == B_OK)
					)
				{
					ChannelKick(qChan, qUser, qWho);
				}
				break;
			}
		case NetClient::ChannelSetTopic:
			{
				QString qChan, qUser, qTopic;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) &&
					(GetStringFromMessage(msg, "topic", qTopic) == B_OK) && 
					(GetStringFromMessage(msg, "channel", qChan) == B_OK)
					)
				{
					ChannelTopic(qChan, qUser, qTopic);
				}
				break;
			}
		case NetClient::ChannelSetPublic:
			{
				QString qChan, qUser;
				bool pub;
				if (
					(GetStringFromMessage(msg, PR_NAME_SESSION, qUser) == B_OK) &&
					(GetStringFromMessage(msg, "channel", qChan) == B_OK) &&
										(msg()->FindBool("public", pub) == B_OK)
					)
				{
					ChannelPublic(qChan, qUser, pub);
				}
				break;
			}
	}
}

void
Channels::ServerConnected()
{
	Create->setEnabled(true);
}

void
Channels::ServerDisconnected()
{
	Create->setEnabled(false);
	Join->setEnabled(false);
}

void
Channels::ChannelSelected()
{
	if (fNetClient->IsConnected())
	{
		if (ChannelList->selectedItem() != NULL)
		{
			QString channel = ChannelList->selectedItem()->text(0);
			ChannelInfo * info;
			if (fChannels.GetValue(channel, info) == B_OK)
			{
				if (!info->FindUser(gWin->GetUserID()))
				{
					Join->setEnabled(true);
					return;
				}
			}
		}
		Join->setEnabled(false);
	}
}