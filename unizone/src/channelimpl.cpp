#include <qapplication.h>

#include "channelimpl.h"
#include "global.h"
#include "wpwevent.h"
#include "gotourl.h"
#include "settings.h"
#include "formatting.h"
#include "nicklist.h"

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
		fActive = ((Channels *) parent)->IsPublic(fName);
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
	

	connect(fText, SIGNAL(URLClicked()), this, SLOT(URLClicked()));
	connect(fText, SIGNAL(highlighted(const QString&)), this,
			SLOT(URLSelected(const QString&)));
	connect(fChat, SIGNAL(TabPressed(QString)), this, SLOT(TabPressed(QString)));
#ifndef WIN32
	connect(fText, SIGNAL(GotShown(const QString &)), this, SLOT(GotShown(const QString &)));
#endif
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
	((Channels *) fParent)->PartChannel(fName);
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
		((Channels *) fParent)->SetTopic(fName, topic);
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
		((Channels *) fParent)->SetPublic(fName, p);
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
	((Channels *) fParent)->PartChannel(fName, user);
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
Channel::URLClicked()
{
	if (fURL != QString::null)
	{
		// <postmaster@raasu.org> 20021021 -- Use lower() to eliminate not matching because of mixed casing
		if (fURL.lower().startsWith("beshare:") || fURL.lower().startsWith("share:"))
		{
			QString surl = fURL.mid(fURL.find(":")+1);
			WinShareWindow::LaunchSearch(surl);
		}
		else
			GotoURL(fURL);
	}
}

void
Channel::GotShown(const QString & txt)
{
#ifndef WIN32
	fText->setText(WinShareWindow::ParseForShown(txt));
#endif
}

void
Channel::URLSelected(const QString & href)
{
	fURL = href;
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
				// Change channel topic
				if (wte->Text().lower().startsWith("/topic "))
				{
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
						((Channels *) fParent)->AddAdmin(fName, user);
					}
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));
				}
				// Remove admin
				else if (wte->Text().lower().startsWith("/deop "))
				{
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
						((Channels *) fParent)->RemoveAdmin(fName, user);
					}
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));
				}
				// Invite or Kick users
				else if (wte->Text().lower().startsWith("/invite "))
				{
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
						WUserRef uref = gWin->FindUser(qTemp);
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
						if (!talking)	// user not yet in list? add
							Invite(uref()->GetUserID());	// the EASY way :)
					}
					else if (gWin->fSettings->GetError())
						PrintError(tr("No users passed."));			
				}
				else if (wte->Text().lower().startsWith("/kick "))
				{
					if ( !((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
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
						WUserRef uref = gWin->FindUser(qTemp);
						for (WUserIter uit = fUsers.begin(); uit != fUsers.end(); uit++)
						{
							WUserRef found = (*uit).second;
							if (found() == uref())
							{
								Kick(uref()->GetUserID());
								
								f = true;
								break;	// done...
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
					PRINT("\t\t%S\n", qStringToWideChar(message));
					SendChannelText(message);
				}
				else if (wte->Text().lower().startsWith("/action ") ||
						wte->Text().lower().startsWith("/me "))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += " ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					PRINT("\t\t%S\n", qStringToWideChar(message));
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
		output = WinShareWindow::GetTimeStamp();
	output += str;

	CheckScrollState();
	fText->append(output);
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
		WUserRef uref = gWin->FindUser(user);
		QString name = uref()->GetUserName();
		name = FixStringStr(name);
		QString message = FixStringStr(text);
		QString fmt;
		fmt = WFormat::LocalName.arg(WColors::RemoteName).arg(gWin->fSettings->GetFontSize()).arg(user).arg(name);
		fmt += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(message);
		PrintText(fmt);
	}
}

void
Channel::Action(const QString & name, const QString & msg, bool batch)
{
	QString nameText = FixStringStr(msg);
	if (gWin->NameSaid(nameText))
		QApplication::beep();

	QString chat = WFormat::Action().arg(WColors::Action).arg(gWin->fSettings->GetFontSize());
	chat += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(tr("%1 %2").arg(FixStringStr(name)).arg(nameText));
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
	if ( ((Channels *) fParent)->IsAdmin(fName, fNet->LocalSessionID()) )
	{
		SetTopic( fTopicEdit->text() );
	}	
}

void
Channel::SetActive(bool a)
{
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
		QString admins = ((Channels *) fParent)->GetAdmins(fName);
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