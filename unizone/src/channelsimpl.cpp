#include "channelsimpl.h"
#include "tokenizer.h"
#include "global.h"
#include "debugimpl.h"

#include <qinputdialog.h>
#include <qmessagebox.h>

Channels::Channels(QWidget* parent, NetClient * net, const char* name, bool modal, WFlags fl)
: ChannelsBase(parent, name, modal, /* QDialog::WDestructiveClose |*/ QWidget::WStyle_Minimize | 
			  QWidget::WStyle_Maximize | QWidget::WStyle_Title | QWidget::WStyle_SysMenu /* flags */)
{
	if ( !name )
	{
		setName( "Channels" );
	}
	fNet = net;
	// Net Client events
	connect( 
		fNet, SIGNAL(ChannelAdded(const QString, const QString, int64)), 
		this, SLOT(ChannelAdded(const QString, const QString, int64)) 
		);
	connect(
		fNet, SIGNAL(ChannelTopic(const QString, const QString, const QString)),
		this, SLOT(ChannelTopic(const QString, const QString, const QString))
		);
	connect(
		fNet, SIGNAL(ChannelOwner(const QString, const QString, const QString)),
		this, SLOT(ChannelOwner(const QString, const QString, const QString))
		);
	connect(
		fNet, SIGNAL(ChannelPublic(const QString, const QString, bool)),
		this, SLOT(ChannelPublic(const QString, const QString, bool))
		);
	connect(
		fNet, SIGNAL(ChannelAdmins(const QString, const QString, const QString)),
		this, SLOT(ChannelAdmins(const QString, const QString, const QString))
		);
	connect(
		fNet, SIGNAL(UserIDChanged(QString, QString)),
		this, SLOT(UserIDChanged(QString, QString))
		);
	// Remote event signals
	connect(
		parent, SIGNAL(ChannelCreated(const QString, const QString, int64)),
		this, SLOT(ChannelCreated(const QString, const QString, int64))
		);
	connect(
		parent, SIGNAL(ChannelJoin(const QString, const QString)),
		this, SLOT(ChannelJoin(const QString, const QString))
		);
	connect(
		parent, SIGNAL(ChannelPart(const QString, const QString)),
		this, SLOT(ChannelPart(const QString, const QString))
		);
	connect(
		parent, SIGNAL(ChannelInvite(const QString, const QString, const QString)),
		this, SLOT(ChannelInvite(const QString, const QString, const QString))
		);
	connect(
		parent, SIGNAL(ChannelKick(const QString, const QString, const QString)),
		this, SLOT(ChannelKick(const QString, const QString, const QString))
		);
	connect(
		parent, SIGNAL(ChannelTopic(const QString, const QString, const QString)),
		this, SLOT(ChannelTopic(const QString, const QString, const QString))
		);
	connect(
		parent, SIGNAL(ChannelPublic(const QString, const QString, bool)),
		this, SLOT(ChannelPublic(const QString, const QString, bool))
		);
	// Window widget events
	connect(Create, SIGNAL(clicked()), this, SLOT(CreateChannel()));
	connect(Join, SIGNAL(clicked()), this, SLOT(JoinChannel()));

	if (fNet)
	{
		if (fNet->IsInternalThreadRunning())
		{
			QString * qChannels = fNet->GetChannelList();
			int n = fNet->GetChannelCount();
			if (n > 0)
			{
				int i = 0;
				while (i < n)
				{
					int64 now = GetCurrentTime64();
					ChannelCreated(qChannels[i], fNet->LocalSessionID(), now);
					i++;
				}
			}
			delete [] qChannels;
		}
	}
}

Channels::~Channels()
{
	emit Closed();
}

void
Channels::ChannelAdmins(const QString channel, const QString sid, const QString admins)
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
Channels::IsAdmin(QString channel, QString user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->IsAdmin(user);
	}
	return false;
}

bool
Channels::IsOwner(QString channel, QString user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->IsOwner(user);
	}
	return false;
}

bool
Channels::IsPublic(QString channel)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		return (*iter).second->GetPublic();
	}
	return false;
}

void
Channels::ChannelAdded(const QString channel, const QString sid, int64 timecreated)
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
		// Create Window item
		//Channel * win = new Channel(this, fNet, channel);
		//win->SetOwner(sid);
		//win->show();
		//wcp.second->SetWindow(win);
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
	//}
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
	if (!fNet->IsInternalThreadRunning())
	{
		// Don't create, when we aren't connected
		return;
	}

	bool ok = FALSE;
	QString text = QInputDialog::getText( 
		tr( "Create Channel" ), 
		tr( "Please enter channel name" ),
#if (QT_VERSION >= 0x030100)
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
			wcp.second = new ChannelInfo(text, fNet->LocalSessionID());
			fChannels.insert(fChannels.end(), wcp);

			it = fChannels.find(text);
			// Create ListView item
			QListViewItem * item = new QListViewItem(ChannelList, text, "", "", "", "");
			wcp.second->SetItem(item);
			// Create Window item
			Channel * win = new Channel(this, fNet, text);
			wcp.second->SetWindow(win);
			wcp.second->AddUser(fNet->LocalSessionID());
			win->SetOwner(fNet->LocalSessionID());
			win->show();

			//wcp.second->AddAdmin(fNet->LocalSessionID());
			ChannelAdmins(text, fNet->LocalSessionID(), fNet->LocalSessionID());
			UpdateUsers(it);
			UpdatePublic(it);

			// Send Channel Data message
			MessageRef cc = GetMessageFromPool(NetClient::ChannelData);
			if (cc())
			{
				QString to("/*/*/unishare");
				cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
				cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
				cc()->AddInt64("when", GetCurrentTime64());
				cc()->AddString("channel", (const char *) text.utf8());
				fNet->SendMessageToSessions(cc);
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
	//WChannelIter it = fChannels.find(text);
	//if (it != fChannels.end())
	//{
		JoinChannel(text);
	//}
}

void
Channels::JoinChannel(QString channel)
{
	WChannelIter it = fChannels.find(channel);
	Channel * win;
	if (!(*it).second->GetWindow())
	{
		//
		// Create Channel Window
		//
		
		// Create Window item
		win = new Channel(this, fNet, channel);
		(*it).second->SetWindow(win);
	}
	else
	{
		win = (*it).second->GetWindow();
	}
	
	// Send user list to window
	PRINT("JoinChannel: GetUsers = %s\n", (*it).second->GetUsers().latin1());
	int n = fNet->GetUserCount(channel);
	if (n > 0)
	{
		int i = 0;
		QString * user = fNet->GetChannelUsers(channel);
		while (i < n)
		{
			(*it).second->GetWindow()->AddUser(user[i++]);
		}
		delete [] user;
	}
	
	(*it).second->AddUser(fNet->LocalSessionID());
	win->SetOwner((*it).second->GetOwner());
	win->SetTopic((*it).second->GetTopic());
	win->SetPublic((*it).second->GetPublic());
	win->show();
	
	UpdateUsers(it);
	UpdatePublic(it);
	
	if (!(*it).second->GetPublic()) // Needs invite?
	{
		// Send Channel Invite message
		MessageRef cc = GetMessageFromPool(NetClient::ChannelInvite);
		if (cc())
		{
			QString to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddString("who", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", GetCurrentTime64());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNet->SendMessageToSessions(cc);
		}
	}	
}

void
Channels::ChannelCreated(const QString channel, const QString owner, int64 timecreated)
{
	WChannelIter it = fChannels.find(channel);
	if (it == fChannels.end())
	{
		// Create Channel
		WChannelPair wcp;
		wcp.first = channel;
		wcp.second = new ChannelInfo(channel, fNet->LocalSessionID());
		fChannels.insert(fChannels.end(), wcp);

		it = fChannels.find(channel);
		// Create ListView item
		QListViewItem * item = new QListViewItem(ChannelList, channel, "", "", "", "");
		wcp.second->SetItem(item);
		// Create Window item
		//Channel * win = new Channel(this, fNet, channel);
		//wcp.second->SetWindow(win);
		//wcp.second->SetCreated(timecreated);
		//win->SetOwner(fNet->LocalSessionID());
		//win->show();
		ChannelAdmins(owner, channel, owner);
		ChannelJoin(channel, owner);
		UpdatePublic(it);
		UpdateUsers(it);
	}
	else if (timecreated >= (*it).second->GetCreated())
	{
		// Send Channel Data message
		MessageRef cc = GetMessageFromPool(NetClient::ChannelKick);
		if (cc())
		{
			QString to("/*/");
			to += owner;
			to += "/unishare";
				cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", (*it).second->GetCreated());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNet->SendMessageToSessions(cc);
		}
	}
	else
	{
		(*it).second->SetCreated(timecreated);
		(*it).second->SetOwner(owner);
	}
}

void
Channels::ChannelJoin(const QString channel, const QString user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->AddUser(user);
		UpdateUsers(iter);
	}
}

void
Channels::ChannelPart(const QString channel, const QString user)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->RemoveUser(user);
		UpdateUsers(iter);
	}
}

void
Channels::PartChannel(QString channel, QString user)
{
	if (user == QString::null)
	{
		user = fNet->LocalSessionID();
		MessageRef cc = GetMessageFromPool(NetClient::ChannelPart);
		if (cc())
		{
			QString to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", GetCurrentTime64());
			cc()->AddString("channel", (const char *) channel.utf8());
			fNet->SendMessageToSessions(cc);
		}
	}

	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		// Make sure we don't have a stale window reference, so we can re-join later
		if (user == fNet->LocalSessionID())
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
Channels::AddAdmin(QString channel, QString user)
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
Channels::RemoveAdmin(QString channel, QString user)
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
Channels::GetAdmins(QString channel)
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
Channels::SetTopic(QString channel, QString topic)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->SetTopic(topic);
		UpdateTopic(iter);
	}
}

void
Channels::SetPublic(QString channel, bool pub)
{
	WChannelIter iter = fChannels.find(channel);
	if (iter != fChannels.end())
	{
		(*iter).second->SetPublic(pub);
		UpdatePublic(iter);
	}
}

void
Channels::ChannelInvite(const QString channel, const QString user, const QString who)
{
	WChannelIter iter = fChannels.find(channel);
	// We need to have existing channel to be able to check for admin status
	if (
		(who == fNet->LocalSessionID()) && 
		(iter == fChannels.end())
		)
	{
		ChannelAdded(channel, user, GetCurrentTime64());
		iter = fChannels.find(channel);
	}

	if (IsAdmin(channel, user))
	{
		if (who == fNet->LocalSessionID())
		{
			// Got invited
			if (!(*iter).second->GetWindow())
			{
				if (QMessageBox::information(this, tr( "Channels" ), 
					tr( "User #%1 invited you to channel %2." " Do you accept?").arg(user).arg(channel),
					tr( "Yes" ), tr( "No" )) == 0)	// 0 is the index of "yes"
				{
					Channel * win = new Channel(this, fNet, channel);
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
		else if ( IsAdmin(channel, fNet->LocalSessionID()) && (*iter).second->GetWindow() )
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
Channels::ChannelKick(const QString channel, const QString user, const QString who)
{
	WChannelIter iter = fChannels.find(channel);
	if (IsAdmin(channel, user))
	{
		if (who == fNet->LocalSessionID())
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
Channels::ChannelTopic(const QString channel, const QString user, const QString topic)
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
Channels::ChannelOwner(const QString channel, const QString user, const QString owner)
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
Channels::ChannelPublic(const QString channel, const QString user, bool pub)
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
Channels::UserIDChanged(QString oldid, QString newid)
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
		PRINT("UserIDChanged, myid = %s, old = %s, new = %s\n", fNet->LocalSessionID().latin1(), oldid.latin1(), newid.latin1());
		if (newid == fNet->LocalSessionID())
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
