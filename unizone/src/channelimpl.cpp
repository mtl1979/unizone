#include <qapplication.h>

#include "channelimpl.h"
#include "global.h"
#include "wpwevent.h"
#include "gotourl.h"
#include "settings.h"
#include "formatting.h"
#include "nicklist.h"
#include "textevent.h"
#include "platform.h"
#include "wstring.h"

Channel::Channel( QWidget* parent, NetClient * net, QString cname, const char* name, bool modal, WFlags fl)
: ChannelBase(/* parent */ NULL, name, modal, QDialog::WDestructiveClose | QWidget::WStyle_Minimize | 
			  QWidget::WStyle_Maximize | QWidget::WStyle_Title | QWidget::WStyle_SysMenu /* flags */)
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
		setCaption( tr("Channel Window - %1").arg(fName) );
		fActive = gWin->IsPublic(fName);
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

	fChannelUsers->addColumn(tr("Name"));
	fChannelUsers->addColumn(tr("ID"));
	fChannelUsers->addColumn(tr("Status"));
	fChannelUsers->addColumn(tr("Files"));
	fChannelUsers->addColumn(tr("Connection"));
	fChannelUsers->addColumn(tr("Load"));
	fChannelUsers->addColumn(tr("Client"));		// as of now... winshare specific, WinShare pings all the users and parses the string for client info

	fChannelUsers->setColumnAlignment(WNickListItem::ID, AlignRight); // <postmaster@raasu.org> 20021005
	fChannelUsers->setColumnAlignment(WNickListItem::Files, AlignRight); // <postmaster@raasu.org> 20021005
	fChannelUsers->setColumnAlignment(WNickListItem::Load, AlignRight); // <postmaster@raasu.org> 20021005

	for (int column = 0; column < 6; column++)
		fChannelUsers->setColumnWidthMode(column, QListView::Manual);

	// set the sort indicator to show
	fChannelUsers->setShowSortIndicator(true);

	fChannelUsers->setAllColumnsShowFocus(true);

	fText = new WHTMLView(fSplitChat);
	CHECK_PTR(fText);

	fChat = new WChatText(this, fSplitChat);
	CHECK_PTR(fChat);

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
	

	connect(fText, SIGNAL(URLClicked(const QString &)), this, SLOT(URLClicked(const QString &)));
	connect(fChat, SIGNAL(TabPressed(QString)), this, SLOT(TabPressed(QString)));
	connect(fText, SIGNAL(GotShown(const QString &)), this, SLOT(GotShown(const QString &)));
	connect(fTopicEdit, SIGNAL(returnPressed()), this, SLOT(UpdateTopic()));

	connect(
		parent, SIGNAL(ChannelAdminsChanged(const QString, const QString)),
		this, SLOT(ChannelAdminsChanged(const QString, const QString))
		);

	connect(gWin, SIGNAL(UpdatePrivateUserLists()), this, SLOT(UpdateUserList()));
	connect(gWin, SIGNAL(NewChannelText(const QString, const QString, const QString)),
			this, SLOT(NewChannelText(const QString, const QString, const QString)));
	connect(fNet, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));

	UpdateNode();
}

Channel::~Channel()
{
	gWin->PartChannel(fName);
}

void
Channel::SetOwner(QString owner)
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
			QString to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			fNet->SendMessageToSessions(cc);
		}
		UpdateNode();
	}
}

void
Channel::SetTopic(QString topic)
{
	if (fTopic != topic)
	{
		fTopic = topic;
		MessageRef cc = GetMessageFromPool(NetClient::ChannelSetTopic);
		if (cc())
		{
			QString to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			cc()->AddString("topic", (const char *) topic.utf8());
			fNet->SendMessageToSessions(cc);
		}
		fTopicEdit->setText(topic);
		UpdateNode();
		PrintSystem( tr( "Topic set to %1." ).arg(fTopicEdit->text() ) );
		gWin->SetTopic(fName, topic);
	}
	
}

void
Channel::SetPublic(bool p)
{
	if (fPublic != p)
	{
		fPublic = p;
		MessageRef cc = GetMessageFromPool(NetClient::ChannelSetTopic);
		if (cc())
		{
			QString to("/*/*/unishare");
			cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
			cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			cc()->AddInt64("when", GetCurrentTime64());
			cc()->AddString("channel", (const char *) fName.utf8());
			cc()->AddBool("public", p);
			fNet->SendMessageToSessions(cc);
			UpdateNode();
		}
		gWin->SetPublic(fName, p);
		PrintSystem( tr("Channel is now %1.").arg(p ? tr( "public" ) : tr( "private" ) ) );
	}	
}

void
Channel::Invite(QString user)
{
	MessageRef cc = GetMessageFromPool(NetClient::ChannelInvite);
	if (cc())
	{
		QString to("/*/");
		to += user;
		to += "/unishare";
		cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
		cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
		cc()->AddInt64("when", GetCurrentTime64());
		cc()->AddString("channel", (const char *) fName.utf8());
		fNet->SendMessageToSessions(cc);
	}
}

void
Channel::Kick(QString user)
{
	MessageRef cc = GetMessageFromPool(NetClient::ChannelKick);
	if (cc())
	{
		QString to("/*/");
		to += user;
		to += "/unishare";
		cc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
		cc()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
		cc()->AddInt64("when", GetCurrentTime64());
		cc()->AddString("channel", (const char *) fName.utf8());
		fNet->SendMessageToSessions(cc);
	}
	gWin->PartChannel(fName, user);
}

void
Channel::TabPressed(QString str)
{
	PRINT("Channel::Tab\n");
	WPWEvent *e = new WPWEvent(WPWEvent::TabComplete, fChat->text());
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
		if (url.lower().startsWith("beshare:") || url.lower().startsWith("share:"))
		{
			QString surl = url.mid(url.find(":") + 1);
			WinShareWindow::LaunchSearch(surl);
		}
		else
			GotoURL(url);
	}
}

void
Channel::GotShown(const QString & txt)
{
	fText->setText(ParseForShown(txt));
}

void
Channel::UpdateUserList()
{
	WUserIter it = fUsers.begin();
	while ( it != fUsers.end())
	{
		(*it).second()->AddToListView(fChannelUsers);
		it++;
	}
}

void
Channel::AddUser(QString user)
{
	if (user == "0")
	{
		return;
	}
	WUserIter it = fUsers.find(user);
	if (it == fUsers.end())
	{
		WUserRef uref = gWin->FindUser(user);
		if (uref())
		{
			WUserPair p = MakePair(user, uref);
			fUsers.insert(p);
			uref()->AddToListView(fChannelUsers);
		}
	}
}

bool
Channel::RemUser(QString user)
{
	WUserIter it = fUsers.find(user);
	if (it == fUsers.end())
	{
		return false;
	}
	(*it).second()->RemoveFromListView(fChannelUsers);
	fUsers.erase(it);
	return true;
}

void
Channel::customEvent(QCustomEvent * event)
{
	PRINT("Channel::customEvent\n");
	switch (event->type())
	{
		case WPWEvent::TabCompleted:
		{
			WPWEvent * we = dynamic_cast<WPWEvent *>(event);
			if (we)
			{
				fChat->setText(we->GetText());
				fChat->setCursorPosition(9999,9999);
			}
			return;
		}

		case WTextEvent::TextType:
		{
			WTextEvent * wte = dynamic_cast<WTextEvent *>(event);
			if (wte)
			{
				// Give a list of available commands
				if (wte->Text().lower().startsWith("/help"))
				{
					QString help =	"\n";
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
					ParseString(help);
					PrintSystem(help);
				}
				// Give a list of channel admins
				else if (wte->Text().lower().startsWith("/listadmins"))
				{
					PrintSystem(tr( "List of channel admins:" ));
					for (WUserIter iter = fUsers.begin(); iter != fUsers.end(); iter++)
					{
						if ( gWin->IsAdmin(fName, (*iter).second()->GetUserID()) )
						{
							PrintSystem( (*iter).second()->GetUserID() + " - " + (*iter).second()->GetUserName() );
						}
					}

				}
				// Change channel topic
				else if (wte->Text().lower().startsWith("/topic "))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					QString qTemp = GetParameterString(wte->Text());
					SetTopic(qTemp);
				}
				// Set Channel to Public mode
				else if (wte->Text().lower().startsWith("/public"))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					
					SetPublic(true);
				}
				// Set Channel to Private mode
				else if (wte->Text().lower().startsWith("/private"))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					
					SetPublic(false);	
				}
				// Add admin
				else if (wte->Text().lower().startsWith("/op "))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					QString user = GetParameterString(wte->Text());
					if (user.length() > 0)
					{
						if (!gWin->IsConnected(user))
						{
							if (gWin->fSettings->GetError())
								PrintError( tr( "User(s) not found!" ) );
							break;
						}
						gWin->AddAdmin(fName, user);
					}
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));
				}
				// Remove admin
				else if (wte->Text().lower().startsWith("/deop "))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					QString user = GetParameterString(wte->Text());
					if (user.length() > 0)
					{
						if (!gWin->IsConnected(user))
						{
							if (gWin->fSettings->GetError())
								PrintError( tr( "User(s) not found!" ) );
							break;
						}
						gWin->RemoveAdmin(fName, user);
					}
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));
				}
				// Invite or Kick users
				else if (wte->Text().lower().startsWith("/invite "))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					QString qTemp = GetParameterString(wte->Text());
					if (qTemp.length() > 0)
					{
						// see if the user is already in the list
						bool talking = false;
						WUserRef uref = FindUser(qTemp);
						if (uref())
						{
							for (WUserIter uit = fUsers.begin(); uit != fUsers.end(); uit++)
							{
								WUserRef found = (*uit).second;
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
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));			
				}
				else if (wte->Text().lower().startsWith("/kick "))
				{
					if ( !gWin->IsAdmin(fName, fNet->LocalSessionID()) )
					{
						// Need to be admin
						if (gWin->fSettings->GetError())
							PrintError( tr( "Not allowed!" ) );
						break;
					}
					QString qTemp = GetParameterString(wte->Text());
					if (qTemp.length() > 0)
					{
						// see if the user is already in the list
						bool f = false;
						WUserRef uref = FindUser(qTemp);
						if (uref())
						{
							for (WUserIter uit = fUsers.begin(); uit != fUsers.end(); uit++)
							{
								WUserRef found = (*uit).second;

								WString wUser1 = found()->GetUserID();
								WString wUser2 = uref()->GetUserID();
								PRINT("found - UserID = %S\n", wUser1.getBuffer());
								PRINT("uref  - UserID = %S\n", wUser2.getBuffer());

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
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));			
				}
				else if (wte->Text().lower().startsWith("/action's ") ||
						wte->Text().lower().startsWith("/me's "))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += "'s ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
					WString wMessage = message;
					PRINT("\t\t%S\n", wMessage.getBuffer());
					
					SendChannelText(message);
				}
				else if (wte->Text().lower().startsWith("/action ") ||
						wte->Text().lower().startsWith("/me "))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += " ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
					WString wMessage = message;
					PRINT("\t\t%S\n", wMessage.getBuffer());
					
					SendChannelText(message);
				}
				else if (wte->Text().lower().startsWith("/clear"))
				{
					fText->setText("");	// empty the text
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
Channel::SendChannelText(QString message)
{
	if (fNet->IsInternalThreadRunning())
	{
		MessageRef chat(GetMessageFromPool(NetClient::ChannelText));
		if (chat())
		{
			QString tostr = "/*/*/unishare";
			chat()->AddString(PR_NAME_KEYS, (const char *) tostr.utf8());
			chat()->AddString("session", (const char *) fNet->LocalSessionID().utf8());
			chat()->AddString("channel", (const char *) fName.utf8());
			chat()->AddString("text", (const char *) message.utf8());
			fNet->SendMessageToSessions(chat);
		}
	}
	QString name = FixStringStr(gWin->GetUserName());
	message = FixStringStr(message);
	QString fmt;
	fmt = WFormat::LocalName.arg(WColors::LocalName).arg(gWin->fSettings->GetFontSize()).arg(fNet->LocalSessionID()).arg(name);
	fmt += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(message);
	PrintText(fmt);
}

void
Channel::PrintText(const QString & str)
{
	QString output("");

	// Check for timestamp
	if (gWin->fSettings->GetTimeStamps())
		output = GetTimeStamp();
	output += str;

	CheckScrollState();
	if (fText->text().isEmpty())
		fText->setText(output);
	else
		fText->append(
#if (QT_VERSION < 0x030100)
				"\t" + 
#endif
				output);
	UpdateView();
}

void
Channel::PrintSystem(const QString & msg)
{
	QString s = WFormat::SystemText().arg(WColors::System).arg(gWin->fSettings->GetFontSize()); // <postmaster@raasu.org> 20021127 -- Wrong order!!
	s += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(ParseChatText((QString &)msg));
	PrintText(s);
}

void
Channel::PrintError(const QString & error)
{
	if (gWin->fSettings->GetError())
	{
		QString e = WFormat::Error().arg(WColors::Error).arg(gWin->fSettings->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(gWin->fSettings->GetFontSize()).arg(error);
		PrintText(e);
	}
}

void
Channel::NewChannelText(const QString channel, const QString user, const QString text)
{
	if (channel != fName)
		return;
	if (!fActive)
		return;
	if ( text.lower().left(6) == "/me's " )
	{
		if ( !gWin->IsIgnored((QString &) user) )
		{
			QString msg = GetParameterString(text);
			if ((msg.length() > 0) && gWin->fSettings->GetChat())
				Action(user + "'s", msg, true);
		}
	}
	else if (text.lower().left(4) == "/me ")
	{
		if ( !gWin->IsIgnored((QString &) user) )
		{
			QString msg = GetParameterString(text);
			if ((msg.length() > 0) && gWin->fSettings->GetChat())
				Action(user, msg, true);
		}
	}
	else
	{
		WUserRef uref = FindUser(user);
		if (uref())
		{
			QString name = uref()->GetUserName();
			name = FixStringStr(name);
			QString message = FixStringStr(text);
			QString fmt;
			fmt = WFormat::LocalName.arg(WColors::RemoteName).arg(gWin->fSettings->GetFontSize()).arg(user).arg(name);
			fmt += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(message);
			PrintText(fmt);
		}
	}
}

void
Channel::Action(const QString & name, const QString & msg, bool batch)
{
	QString nameText = FixStringStr(msg);
	if (gWin->NameSaid(nameText) && gWin->fSettings->GetSounds())
		QApplication::beep();

	QString chat = WFormat::Action().arg(WColors::Action).arg(gWin->fSettings->GetFontSize());
	chat += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(FixStringStr(name) + " " + nameText);
	CheckScrollState();
	PrintText(chat);
}

void
Channel::CheckScrollState()
{
	QScrollBar * scroll = fText->verticalScrollBar();
	PRINT("CHECKSCROLLSTATE: value = %d, maxValue = %d, minValue = %d\n", scroll->value(), scroll->maxValue(), scroll->minValue());
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}

void
Channel::UpdateView()
{
	if (fScrollDown)
		fText->setContentsPos(0, fText->contentsHeight());
#ifndef WIN32
	fText->repaintContents(fText->contentsX(), fText->contentsY(),
					fText->contentsWidth(), fText->contentsHeight(),
					false);
#endif
}

void
Channel::UpdateTopic()
{
	if ( gWin->IsAdmin(fName, fNet->LocalSessionID()) )
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
	MessageRef cc = GetMessageFromPool();
	if (cc())
	{
		QString admins = gWin->GetAdmins(fName);
		cc()->AddString("owner", (const char *) fOwner.utf8());
		cc()->AddString("topic", (const char *) fTopic.utf8());
		cc()->AddString("admins", (const char *) admins.utf8());
		cc()->AddBool("public", fPublic);
		String node = "unishare/channeldata/";
		node += (const char *) fName.utf8();
		fNet->SetNodeValue(node.Cstr(), cc);
	}
}

void
Channel::ChannelAdminsChanged(const QString channel, const QString admins)
{
	if ((channel == fName) && (fStrAdmins != admins))
	{
		UpdateNode();
		fStrAdmins = admins;
	}
}

void
Channel::UserDisconnected(QString sid, QString name)
{
	WUserIter iter = fUsers.find(sid);
	if (iter != fUsers.end())
	{
		if (gWin->fSettings->GetUserEvents())
		{
			QString parse = WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg( 
				WFormat::UserDisconnected().arg(sid).arg(FixStringStr(name)).arg(WColors::RemoteName) 
				); // <postmaster@raasu.org> 20021112
			PrintSystem(parse);
		}
		(*iter).second()->RemoveFromListView(fChannelUsers);
		fUsers.erase(iter);
	}
}

WUserRef
Channel::FindUser(QString user)
{
	for (WUserIter iter = fUsers.begin(); iter != fUsers.end(); iter++)
	{
		if (gWin->MatchUserFilter((*iter).second(), (const char*) user.utf8()))
		{
			return (*iter).second;
		}
	}
	return WUserRef(NULL, NULL);
}
