#include "channelimpl.h"
#include "global.h"
#include "wpwevent.h"
#include "gotourl.h"
#include "settings.h"
#include "formatting.h"
#include "nicklist.h"
#include "textevent.h"
#include "util.h"
#include "wstring.h"
#include "netclient.h"

#include "reflector/StorageReflectConstants.h"

Channel::Channel( QWidget* parent, NetClient * net, QString cname, const char* name, bool modal, WFlags /* fl */)
: ChannelBase(/* parent */ NULL, name, modal, WDestructiveClose | WStyle_Minimize | 
			  WStyle_Maximize | WStyle_Title | WStyle_SysMenu /* flags */),
			  ChatWindow(PrivateType)
{
	if (!name)
	{
		setName( "Channel" );
	}
	fNet = net;
	fName = cname;
	fActive = false;
	fStrAdmins = QString::null;
	fParent = parent;
	if (fName != QString::null)
	{
		QString title = tr("Channel Window - %1").arg(fName);
		setCaption( title );
		fActive = gWin->fChannels->IsPublic(fName);
	}
	fSplit = new QSplitter(Vertical, this);
	CHECK_PTR(fSplit);

	fTopicBox = new QHGroupBox(fSplit);
	CHECK_PTR(fTopicBox);

	fTopicLabel = new QLabel( tr( "Topic:" ), fTopicBox );
	CHECK_PTR(fTopicLabel);

	fTopicEdit = new QLineEdit(fTopicBox);
	CHECK_PTR(fTopicEdit);

	fSplitBottom = new QSplitter(Horizontal, fSplit);
	CHECK_PTR(fSplitBottom);

	fSplitChat = new QSplitter(Vertical, fSplitBottom);
	CHECK_PTR(fSplitChat);

	fChannelUsers = new QListView(fSplitBottom);
	CHECK_PTR(fChannelUsers);

	InitUserList(fChannelUsers);

	fChatText = new WHTMLView(fSplitChat);
	CHECK_PTR(fChatText);

	fInputText = new WChatText(this, fSplitChat);
	CHECK_PTR(fInputText);

	// Set widget size relationships

	QValueList<int> splitList1;
	splitList1.append(1);
	splitList1.append(10);

	fSplit->setSizes(splitList1);

	QValueList<int> splitList2;
	splitList2.append(4);
	splitList2.append(1);

	fSplitBottom->setSizes(splitList2);

	QValueList<int> splitList3;
	splitList3.append(5);
	splitList3.append(1);

	fSplitChat->setSizes(splitList3);
	

	connect(fChatText, SIGNAL(URLClicked(const QString &)), 
			this, SLOT(URLClicked(const QString &)));
	connect(fInputText, SIGNAL(TabPressed(const QString &)), 
			this, SLOT(TabPressed(const QString &)));
	connect(fTopicEdit, SIGNAL(returnPressed()), 
			this, SLOT(UpdateTopic()));

	connect(
		parent, SIGNAL(ChannelAdminsChanged(const QString &, const QString &)),
		this, SLOT(ChannelAdminsChanged(const QString &, const QString &))
		);

	connect(gWin, SIGNAL(NewChannelText(const QString &, const QString &, const QString &)),
			this, SLOT(NewChannelText(const QString &, const QString &, const QString &)));
	connect(fNet, SIGNAL(UserDisconnected(const WUserRef &)), 
			this, SLOT(UserDisconnected(const WUserRef &)));

	if (Settings()->GetLogging())
		StartLogging();

	UpdateNode();
}

Channel::~Channel()
{
	StopLogging();
	gWin->fChannels->PartChannel(fName, fNet->LocalSessionID());
}

void
Channel::SetOwner(const QString & owner)
{
	if (fOwner != owner)
	{
		fOwner = owner;
		MessageRef cc;
		if (fOwner == fNet->LocalSessionID()) // We did create this channel
		{
			cc = GetMessageFromPool(NetClient::ChannelCreated);
		}
		else
		{
			cc = GetMessageFromPool(NetClient::ChannelJoin);
		}
		if (cc())
		{
			String to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, to);
			cc()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			fNet->SendMessageToSessions(cc);
		}
		UpdateNode();
	}
}

void
Channel::SetTopic(const QString & topic)
{
	if (fTopic != topic)
	{
		fTopic = topic;
		MessageRef cc(GetMessageFromPool(NetClient::ChannelSetTopic));
		if (cc())
		{
			String to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, to);
			cc()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			cc()->AddString("topic", (const char *) topic.utf8());
			fNet->SendMessageToSessions(cc);
		}
		fTopicEdit->setText(topic);
		UpdateNode();
		PrintSystem( tr( "Topic set to %1." ).arg(fTopicEdit->text() ) );
		gWin->fChannels->SetTopic(fName, topic);
	}
	
}

void
Channel::SetPublic(bool p)
{
	if (fPublic != p)
	{
		fPublic = p;
		MessageRef cc(GetMessageFromPool(NetClient::ChannelSetTopic));
		if (cc())
		{
			String to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, to);
			cc()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", (int64) GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			cc()->AddBool("public", p);
			fNet->SendMessageToSessions(cc);
			UpdateNode();
		}
		gWin->fChannels->SetPublic(fName, p);
		PrintSystem( tr("Channel is now %1.").arg(p ? tr( "public" ) : tr( "private" ) ) );
	}	
}

void
Channel::Invite(const QString & user)
{
	MessageRef cc(GetMessageFromPool(NetClient::ChannelInvite));
	if (cc())
	{
		QString to("/*/");
		to += user;
		to += "/unishare";
		cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
		cc()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
		cc()->AddInt64("when", (int64) GetCurrentTime64());
		cc()->AddString("channel", (const char *) fName.utf8());
		fNet->SendMessageToSessions(cc);
	}
}

void
Channel::Kick(const QString & user)
{
	MessageRef cc(GetMessageFromPool(NetClient::ChannelKick));
	if (cc())
	{
		QString to("/*/");
		to += user;
		to += "/unishare";
		cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
		cc()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
		cc()->AddInt64("when", (int64) GetCurrentTime64());
		cc()->AddString("channel", (const char *) fName.utf8());
		fNet->SendMessageToSessions(cc);
	}
	gWin->fChannels->PartChannel(fName, user);
}

void
Channel::TabPressed(const QString & /* str */)
{
	PRINT("Channel::Tab\n");
	WPWEvent *e = new WPWEvent(WPWEvent::TabComplete, fInputText->text());
	if (e)
	{
		e->SetSendTo(this);
		QApplication::postEvent(gWin, e);
	}
}

void
Channel::URLClicked(const QString & url)
{
	if (url != QString::null)
	{
		// <postmaster@raasu.org> 20021021 -- Use lower() to eliminate not matching because of mixed casing
		if (startsWith(url, "beshare:", false) || startsWith(url,"share:", false))
		{
			QString surl = url.mid(url.find(":") + 1);
			WinShareWindow::LaunchSearch(surl);
		}
		else
			GotoURL(url);
	}
}

void
Channel::AddUser(const QString & user)
{
	bool ok;
	uint32 uid = user.toULong(&ok);
	if (ok)
	{
		WUserRef uref;
		uref = gWin->FindUser(user);
		if (uref() && !uref()->IsBot())
		{
			fUsers.Put(uid, uref);
			uref()->AddToListView(fChannelUsers);
		}
	}
}

bool
Channel::RemUser(const QString & user)
{
	bool ok;
	uint32 uid = user.toULong(&ok);
	if (ok)
	{
		WUserRef uref;
		if (fUsers.GetValue(uid, uref) == B_ERROR)
		{
			return false;
		}
		uref()->RemoveFromListView(fChannelUsers);
		fUsers.Remove(uid);
		return true;
	}
	else
		return false;
}

void
Channel::customEvent(QCustomEvent * event)
{
	PRINT("Channel::customEvent\n");
	switch ((int) event->type())
	{
		case WPWEvent::TabCompleted:
		{
			WPWEvent * we = dynamic_cast<WPWEvent *>(event);
			if (we)
			{
				fInputText->setText(we->GetText());
				fInputText->gotoEnd();
			}
			return;
		}

		case WTextEvent::TextType:
		{
			WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
			if (wte)
			{
				// Give a list of available commands
				if (CompareCommand(wte->Text(), "/help"))
				{
					QString help;
					help		+=	tr("Channel command reference:");
					help		+=	"\n\n\t\t\t\t";
					help		+=	tr("/action [action] - do something");
					help		+=	"\n\t\t\t\t";
					help		+=	tr("/clear - clear channel window");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/deop [name or session id] - take admin status from other user");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/help - show command reference");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/invite [name or session id] - invite user to channel");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/kick [name or session id] - kick user off the channel");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/listadmins - show channel admins");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/me [action] - same as /action");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/op [name or session id] - give admin status to other user");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/private - set channel to private mode");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/public - set channel to public mode");
					help		+=	"\n\t\t\t\t"; 
					help		+=	tr("/topic [topic] - change the channel topic");
					PrintSystem(ParseString(help));
				}
				// Give a list of channel admins
				else if (CompareCommand(wte->Text(), "/listadmins"))
				{
					PrintSystem(tr( "List of channel admins:" ));
					WUserIter iter = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
					while (iter.HasMoreValues())
					{
						WUserRef uref;
						iter.GetNextValue(uref);
						if ( gWin->fChannels->IsAdmin(fName, uref()->GetUserID()) )
						{
							PrintSystem( uref()->GetUserID() + " - " + uref()->GetUserName() );
						}
					}

				}
				// Change channel topic
				else if (CompareCommand(wte->Text(), "/topic"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						QString qTemp = GetParameterString(wte->Text());
						SetTopic(qTemp);
					}
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				// Set Channel to Public mode
				else if (CompareCommand(wte->Text(), "/public"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
						SetPublic(true);
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				// Set Channel to Private mode
				else if (CompareCommand(wte->Text(), "/private"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
						SetPublic(false);	
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}				
				}
				// Add admin
				else if (CompareCommand(wte->Text(), "/op"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						QString user = GetParameterString(wte->Text());
						if (user.isEmpty())
						{
							if (gWin->fSettings->GetError())
								PrintError(tr("No users passed."));
						}
						else
						{
							if (gWin->IsConnected(user))
								gWin->fChannels->AddAdmin(fName, user);
							else
							{
								if (gWin->fSettings->GetError())
									PrintError( tr( "User(s) not found!" ) );
							}
						}
					}
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				// Remove admin
				else if (CompareCommand(wte->Text(), "/deop"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						QString user = GetParameterString(wte->Text());
						if (user.isEmpty())
						{
							if (gWin->fSettings->GetError())
								PrintError(tr("No users passed."));
						}
						else
						{
							if (gWin->IsConnected(user))
								gWin->fChannels->RemoveAdmin(fName, user);
							else
							{
								if (gWin->fSettings->GetError())
									PrintError( tr( "User(s) not found!" ) );
							}
						}
					}
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				// Invite or Kick users
				else if (CompareCommand(wte->Text(), "/invite"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						QString qTemp = GetParameterString(wte->Text());
						if (qTemp.isEmpty())
						{
							if (gWin->fSettings->GetError())
								PrintError(tr("No users passed."));	
						}
						else
						{
							// see if the user is already in the list
							bool talking = false;
							WUserRef uref = FindUser(qTemp);
							if (uref())
							{
								WUserIter uit = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
								while ( uit.HasMoreValues())
								{
									WUserRef found;
									uit.GetNextValue(found);
									if (found() == uref())
									{
										if (gWin->fSettings->GetUserEvents())
											PrintError(tr("User #%1 (a.k.a %2) is already in this channel window!").arg(uref()->GetUserID()).arg(uref()->GetUserName()));
										
										talking = true;
										break;	// done...
									}
								}
							}
							if (!uref())
							{
								uref = gWin->FindUser(qTemp);
							}
							if (!talking && uref())	// user not yet in list? add
							{
								Invite(uref()->GetUserID());	// the EASY way :)
							}
						}
					}
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				else if (CompareCommand(wte->Text(), "/kick"))
				{
					if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						QString qTemp = GetParameterString(wte->Text());
						if (qTemp.isEmpty())
						{												
							if (gWin->fSettings->GetError())
								PrintError(tr("No users passed."));
						}
						else
						{
							// see if the user is already in the list
							bool f = false;
							WUserRef uref = FindUser(qTemp);
							if (uref())
							{
								WUserIter uit = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
								while (uit.HasMoreValues())
								{
									WUserRef found;
									uit.GetNextValue(found);
									
#ifdef _DEBUG
									WString wuid1(found()->GetUserID());
									WString wuid2(uref()->GetUserID());
									PRINT("found - UserID = %S\n", wuid1.getBuffer());
									PRINT("uref  - UserID = %S\n", wuid2.getBuffer());
#endif
									
									if (found()->GetUserID() == uref()->GetUserID())
									{
										Kick(uref()->GetUserID());
										
										f = true;
										break;	// done...
									}
								}
							}
							if (!f)
							{
								if (gWin->fSettings->GetError())
									PrintError( tr( "User(s) not found!" ) );
							}
						}
					}	
					else
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
					}
				}
				else if (CompareCommand(wte->Text(), "/action's") ||
						CompareCommand(wte->Text(), "/me's"))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += "'s ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
#ifdef _DEBUG
					WString wmessage(message);
					PRINT("\t\t%S\n", wmessage.getBuffer());
#endif
					
					SendChannelText(message);
				}
				else if (CompareCommand(wte->Text(), "/action") ||
						CompareCommand(wte->Text(), "/me"))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += " ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
#ifdef _DEBUG
					WString wmessage(message);
					PRINT("\t\t%S\n", wmessage.getBuffer());
#endif
					
					SendChannelText(message);
				}
				else if (CompareCommand(wte->Text(), "/clear"))
				{
					fChatText->clear();	// empty the text
				}
				else
				{
					SendChannelText(wte->Text());
				}

			}
			return;
		}
	}		
}

void
Channel::SendChannelText(const QString & message)
{
	if (fNet->IsConnected())
	{
		MessageRef chat(GetMessageFromPool(NetClient::ChannelText));
		if (chat())
		{
			String tostr("/*/*/unishare");
			chat()->AddString(PR_NAME_KEYS, tostr);
			chat()->AddString(PR_NAME_SESSION, (const char *) fNet->LocalSessionID().utf8());
			chat()->AddString("channel", (const char *) fName.utf8());
			chat()->AddString("text", (const char *) message.utf8());
			fNet->SendMessageToSessions(chat);
		}
	}
	QString fmt = WFormat::LocalText(fNet->LocalSessionID(), FixString(gWin->GetUserName()), FixString(message));
	PrintText(fmt);
}

void
Channel::NewChannelText(const QString &channel, const QString &user, const QString &text)
{
	if (channel != fName)
		return;
	if (!fActive)
		return;
	if ( CompareCommand(text, "/me's") )
	{
		if ( !gWin->IsIgnored(user) )
		{
			QString msg = GetParameterString(text);
			if (!msg.isEmpty() && gWin->fSettings->GetChat())
				Action(user + "'s", msg);
		}
	}
	else if ( CompareCommand(text, "/me ") )
	{
		if ( !gWin->IsIgnored((QString &) user) )
		{
			QString msg = GetParameterString(text);
			if (!msg.isEmpty() && gWin->fSettings->GetChat())
				Action(user, msg);
		}
	}
	else
	{
		WUserRef uref = FindUser(user);
		if (uref())
		{
			// NOTE: Can't use FixString(text)
			QString fmt = WFormat::RemoteText(user, FixString(uref()->GetUserName()), FormatNameSaid(text));
			PrintText(fmt);
		}
	}
}

void
Channel::UpdateTopic()
{
	if ( gWin->fChannels->IsAdmin(fName, fNet->LocalSessionID()) )
	{
		SetTopic( fTopicEdit->text() );
	}	
}

void
Channel::SetActive(bool a)
{
	if ((fActive) && (!a))
	{
		PrintError(tr( "You got kicked off from this channel!" ));
	}
	fActive = a;
}

void
Channel::resizeEvent(QResizeEvent * e)
{
	fSplit->resize(e->size().width(), e->size().height());
}

void
Channel::UpdateNode()
{
	MessageRef cc(GetMessageFromPool());
	if (cc())
	{
		QString admins = gWin->fChannels->GetAdmins(fName);
		cc()->AddString("owner", (const char *) fOwner.utf8());
		cc()->AddString("topic", (const char *) fTopic.utf8());
		cc()->AddString("admins", (const char *) admins.utf8());
		cc()->AddBool("public", fPublic);
		QString node = "unishare/channeldata/";
		node += fName;
		fNet->SetNodeValue((const char *) node.utf8(), cc);
	}
}

void
Channel::ChannelAdminsChanged(const QString &channel, const QString &admins)
{
	if ((channel == fName) && (fStrAdmins != admins))
	{
		fStrAdmins = admins;
		UpdateNode();
	}
}

void
Channel::UserDisconnected(const WUserRef &user)
{
	QString sid = user()->GetUserID();

	bool ok;
	uint32 uid = sid.toULong(&ok);
	if (ok)
	{
		if (fUsers.ContainsKey(uid))
		{
			if (gWin->fSettings->GetUserEvents())
			{
				QString msg = WFormat::UserDisconnected(sid, FixString(user()->GetUserName())); 
				PrintSystem(msg);
			}
			user()->RemoveFromListView(fChannelUsers);
			fUsers.Remove(uid);
			gWin->fChannels->PartChannel(fName, sid);
		}
	}
}

WUserRef
Channel::FindUser(const QString & user)
{
	bool ok;
	uint32 uid = user.toULong(&ok);
	if (ok)
	{
		WUserRef found;
		if (fUsers.GetValue(uid, found) == B_NO_ERROR)
		{
			if (gWin->MatchUserFilter(found, user))
			{
				return found;
			}
		}
	}
	return WUserRef(NULL);
}

QWidget *
Channel::Window()
{
	return this;
}

void
Channel::LogString(const QString &str)
{
	fLog.LogString(str, true);
}

void
Channel::LogString(const char * str)
{
	fLog.LogString(str, true);
}

void
Channel::StartLogging()
{
	fLog.Create(WLog::LogChannel, fName);	// create a channel log
	if (!fLog.InitCheck())
	{
		if (gWin->fSettings->GetError())
			PrintError( tr( "Failed to create channel log." ) );
	}
}

void
Channel::StopLogging()
{
	fLog.Close();
}


