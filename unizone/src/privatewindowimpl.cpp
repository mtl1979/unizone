#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#define LT WLog::LogType
#else
#define LT WLog
#endif

#include "privatewindowimpl.h"
#include "gotourl.h"
#include "formatting.h"
#include "textevent.h"
#include "global.h"
#include "settings.h"
#include "util/String.h"
#include "platform.h"
#include "util.h"
#include "wstring.h"
#include "wpwevent.h"
#include "nicklist.h"
#include "netclient.h"

#include <qapplication.h>
#include <qmessagebox.h>

/* 
 *  Constructs a privatewindow which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WPrivateWindow::WPrivateWindow(QObject * owner, NetClient * net, QWidget* parent,  const char* name, bool modal, WFlags fl)
    : WPrivateWindowBase(parent, name, modal, fl),
	ChatWindow(PrivateType), fLock(true)
{
	fOwner = owner;
	fNet = net;
	fEncrypted = false;

	if ( !name ) 
		setName( "WPrivateWindow" );
	// start GUI
	fSplit = new QSplitter(this);
	CHECK_PTR(fSplit);

	// setup chat part
	fSplitChat = new QSplitter(fSplit);
	CHECK_PTR(fSplitChat);

	fChatText = new WHTMLView(fSplitChat);
	CHECK_PTR(fChatText);
	// we still want to autocomplete ALL names, not just
	// the one's with the ppl we talk to
	fInputText = new WChatText(this, fSplitChat);
	CHECK_PTR(fInputText);

	// user list
	fPrivateUsers = new QListView(fSplit);
	CHECK_PTR(fPrivateUsers);

	fPrivateUsers->addColumn(tr("Name"));
	fPrivateUsers->addColumn(tr("ID"));
	fPrivateUsers->addColumn(tr("Status"));
	fPrivateUsers->addColumn(tr("Files"));
	fPrivateUsers->addColumn(tr("Connection"));
	fPrivateUsers->addColumn(tr("Load"));
	fPrivateUsers->addColumn(tr("Client"));		// as of now... winshare specific, WinShare pings all the users and parses the string for client info

	fPrivateUsers->setColumnAlignment(WNickListItem::ID, AlignRight); // <postmaster@raasu.org> 20021005
	fPrivateUsers->setColumnAlignment(WNickListItem::Files, AlignRight); // <postmaster@raasu.org> 20021005
	fPrivateUsers->setColumnAlignment(WNickListItem::Load, AlignRight); // <postmaster@raasu.org> 20021005

	for (int column = 0; column < 6; column++)
		fPrivateUsers->setColumnWidthMode(column, QListView::Manual);

	// set the sort indicator to show
	fPrivateUsers->setShowSortIndicator(true);

	fPrivateUsers->setAllColumnsShowFocus(true);

	QValueList<int> splitList;
	splitList.append(4);
	splitList.append(1);

	fSplit->setSizes(splitList);
	fSplitChat->setSizes(splitList);
	fSplitChat->setOrientation(QSplitter::Vertical);

	// create popup menu
	fPopup = new QPopupMenu(this);	// have it deleted on destruction of window
	CHECK_PTR(fPopup);

	connect(fPopup, SIGNAL(activated(int)), this, SLOT(PopupActivated(int)));
	connect(fPrivateUsers, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
			this, SLOT(RightButtonClicked(QListViewItem *, const QPoint &, int)));

	connect(fNet, SIGNAL(UserDisconnected(const QString &, const QString &)), 
			this, SLOT(UserDisconnected(const QString &, const QString &)));
	connect(fNet, SIGNAL(DisconnectedFromServer()), 
			this, SLOT(DisconnectedFromServer()));
	connect(fChatText, SIGNAL(URLClicked(const QString &)), 
			this, SLOT(URLClicked(const QString &)));
	connect(fInputText, SIGNAL(TabPressed(const QString &)), 
			this, SLOT(TabPressed(const QString &)));
	connect(owner, SIGNAL(UpdatePrivateUserLists()), this, SLOT(UpdateUserList()));

#ifdef WIN32
	FindWindowHandle( tr("Private") );
#endif

	if (Settings()->GetLogging())
		StartLogging();
}

/*
 *  Destroys the object and frees any allocated resources
 */
WPrivateWindow::~WPrivateWindow()
{
	StopLogging();
    // no need to delete child widgets, Qt does it all for us
	fLock.lock();
	for (WUserIter it = fUsers.begin(); it != fUsers.end(); it++)
		(*it).second()->RemoveFromListView(fPrivateUsers);
	fLock.unlock();
	WPWEvent *closed = new WPWEvent(WPWEvent::Closed, "");
	if (closed)
	{
		closed->SetSendTo(this);
		QApplication::postEvent(fOwner, closed);
	}
}

void
WPrivateWindow::DisconnectedFromServer()
{
	PRINT("WPrivateWindow::Disconnected\n");
	fUsers.clear();
	if (Settings()->GetError())
		PrintError(tr("Disconnected from server."));
}

void
WPrivateWindow::UserDisconnected(const QString &sid, const QString &name)
{
	WUserIter iter = fUsers.find(sid);
	if (iter != fUsers.end())
	{
		if (Settings()->GetUserEvents())
		{
			QString uname = FixStringStr(name);
			QString msg;
			if (uname.isEmpty())
			{
				msg = WFormat::UserDisconnected2(sid);
			}
			else
			{
				// <postmaster@raasu.org> 20021112
				msg = WFormat::UserDisconnected(sid, uname); 
			}
			QString parse = WFormat::Text(msg);
			PrintSystem(parse);
		}
		(*iter).second()->RemoveFromListView(fPrivateUsers);
		fUsers.erase(iter);

		CheckEmpty();
	}
}

void
WPrivateWindow::URLClicked(const QString & url)
{
	if (url != QString::null)
	{
		QString surl;
		// <postmaster@raasu.org> 20021021 -- Use lower() to eliminate not matching because of mixed casing
		if (url.lower().startsWith("beshare:") || url.lower().startsWith("share:"))
		{
			surl = url.mid(url.find(":") + 1);
			WinShareWindow::LaunchSearch(surl);
		}
		else if (url.lower().startsWith("ttp://"))	// <postmaster@raasu.org> 20030911
		{
			surl = url.mid(url.find(":") + 3);		// skip ://
			WinShareWindow::QueueFile(surl);
		}
		else
			GotoURL(url);
	}
}

void
WPrivateWindow::PutChatText(const QString & fromsid, const QString & message)
{
	WUserIter it = fUsers.find(fromsid);

	if (it != fUsers.end())
	{
		if (Settings()->GetPrivate())
		{
			QString msg = FixStringStr(message);
			QString name = (*it).second()->GetUserName();
			FixString(name);
			if (NameSaid(msg) && Settings()->GetSounds())
				QApplication::beep();
			QString s;
			if ( IsAction(msg, name) ) // simulate action?
			{
				s = WFormat::Action(msg);
			}
			else
			{
				s = WFormat::ReceivePrivMsg(fromsid, name, msg);
			}
			PrintText(s);
			if (Settings()->GetSounds())
				QApplication::beep();
			// <postmaster@raasu.org> 20021021 -- Fix Window Flashing on older API's
#ifdef WIN32
			// flash away!
			if (fWinHandle && !this->isActiveWindow() && (Settings()->GetFlash() & WSettings::FlashPriv))	// got the handle... AND not active? AND user wants us to flash
			{
				WFlashWindow(fWinHandle); // flash
			}
#endif // WIN32
		}
	}
}


void
WPrivateWindow::AddUser(const WUserRef & user)
{
	fLock.lock();
	WUserIter it = fUsers.find(user()->GetUserID());
	if (it == fUsers.end())
	{
		WUserPair p = MakePair(user()->GetUserID(), user);
		fUsers.insert(p);
		user()->AddToListView(fPrivateUsers);
	}
	fLock.unlock();
}

bool
WPrivateWindow::RemUser(const WUserRef & user)
{
	fLock.lock();
	WUserIter it = fUsers.find(user()->GetUserID());
	if (it == fUsers.end())
	{
		fLock.unlock();
		return false;
	}
	(*it).second()->RemoveFromListView(fPrivateUsers);
	fUsers.erase(it);
	fLock.unlock();
	return true;
}

void
WPrivateWindow::TabPressed(const QString & /* str */)
{
	PRINT("WPrivateWindow::Tab\n");
	WPWEvent *e = new WPWEvent(WPWEvent::TabComplete, fInputText->text());
	if (e)
	{
		e->SetSendTo(this);
		QApplication::postEvent(fOwner, e);
	}
}

void
WPrivateWindow::customEvent(QCustomEvent * event)
{
	PRINT("WPrivateWindow::customEvent\n");
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
				QString stxt(wte->Text());
				if (CompareCommand(stxt, "/adduser") ||
					CompareCommand(stxt, "/removeuser"))
				{
					bool rem = stxt.lower().startsWith("/adduser ") ? false : true;

					String targetStr, restOfString;
					WUserSearchMap sendTo;
					QString qTemp = GetParameterString(wte->Text());

					if (WinShareWindow::ParseUserTargets(qTemp, sendTo, targetStr, restOfString, fNet))
					{
						// got some users
						WUserSearchIter iter = sendTo.begin();
						WUserRef user;
						if (sendTo.empty())
						{
							if (Settings()->GetError())
								PrintError( tr( "User(s) not found!" ) );
						}
						else
						{
							while (iter != sendTo.end())
							{
								user = (*iter).second.user;
								QString sid = user()->GetUserID();

								// see if the user is a bot...
								if (user()->IsBot())
								{
									if (Settings()->GetError())
										PrintError(WFormat::PrivateIsBot(sid, user()->GetUserName()));
									iter++;
									continue;	// go on to next user
								}

								if (!rem)	// add a new user
								{
									// see if the user is already in the list
									bool talking = false;
									for (WUserIter uit = fUsers.begin(); uit != fUsers.end(); uit++)
									{
										WUserRef found = (*uit).second;
										if (found()->GetUserID() == sid)
										{
											if (Settings()->GetUserEvents())
												PrintError(tr("User #%1 (a.k.a %2) is already in this private window!").arg(sid).arg(user()->GetUserName()));

											talking = true;
											break;	// done...
										}
									}
									if (!talking)	// user not yet in list? add
										AddUser(user);	// the EASY way :)
								}
								else
								{
									WUserIter foundIt = fUsers.find(user()->GetUserID());	// get the user
									if (foundIt != fUsers.end())
									{
										(*foundIt).second()->RemoveFromListView(fPrivateUsers);
										if (Settings()->GetUserEvents())
										{
											PrintSystem(WFormat::PrivateRemoved((*foundIt).second()->GetUserID(), (*foundIt).second()->GetUserName()));
										}
										fUsers.erase(foundIt);
									}
								}
								iter++;
							}
						}
						if (rem)	// check to see whether we have an empty list
						{
							CheckEmpty();
						}
					}
				}
				else if (CompareCommand(stxt, "/action's") ||
						CompareCommand(stxt, "/me's "))
				{
					QString message = gWin->GetUserName();
					message += "'s ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
#ifdef _DEBUG
					WString wMessage(message);
					PRINT("\t\t%S\n", wMessage.getBuffer());
#endif
					
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, message);
					if (e)
					{
						e->SetSendTo(this);
						QApplication::postEvent(fOwner, e);
					}
				}
				else if (CompareCommand(stxt, "/action") ||
						CompareCommand(stxt, "/me"))
				{
					QString message = gWin->GetUserName();
					message += " ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					
#ifdef _DEBUG
					WString wMessage(message);
					PRINT("\t\t%S\n", wMessage.getBuffer());
#endif
					
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, message);
					if (e)
					{
						e->SetSendTo(this);
						QApplication::postEvent(fOwner, e);
					}
				}
				else if (CompareCommand(stxt, "/clear"))
				{
					fChatText->setText("");	// empty the text
				}
				else if (CompareCommand(stxt, "/encryption"))
				{
					QString qtext = GetParameterString(stxt);
					if (qtext == "on")
					{
						PrintSystem(tr("Encryption enabled."));
						fEncrypted = true;
					}
					else
					{
						fEncrypted = false;
						PrintSystem(tr("Encryption disabled."));
					}
				}
				else
				{
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, stxt, fEncrypted);
					if (e)
					{
						e->SetSendTo(this);
						QApplication::postEvent(fOwner, e);
					}
				}

			}
			return;
		}

		case WPWEvent::TextPosted:
		{
			WPWEvent * we = dynamic_cast<WPWEvent *>(event);
			// we won't get a reply to "TextType" unless we wanted it
			if (we)
			{
				PrintText(we->GetText());
			}
			return;
		}
		
	}		
}

void
WPrivateWindow::resizeEvent(QResizeEvent * e)
{
	fSplit->resize(e->size().width(), e->size().height());
}

void
WPrivateWindow::StartLogging()
{
	fLog.Create(LT::LogPrivate);	// create a private chat log
	if (!fLog.InitCheck())
	{
		if (Settings()->GetError())
			PrintError( tr( "Failed to create private log." ) );
	}
}

void
WPrivateWindow::StopLogging()
{
	fLog.Close();
}

void
WPrivateWindow::RightButtonClicked(QListViewItem * i, const QPoint & p, int /* c */)
{
	// empty menu
	while (fPopup->count() > 0)
		fPopup->removeItemAt(0);
	if (i)
	{
		QString uid = i->text(1);		// session ID
		WUserIter it = fNet->Users().begin();
		while (it != fNet->Users().end())
		{
			if ((*it).second()->GetUserID() == uid)
			{
				// found user...
				// <postmaster@raasu.org> 20021127 -- Remove user from private window
				// <postmaster@raasu.org> 20020924 -- Added ',1'
				fPopup->insertItem(tr("Remove"), 1);
				// <postmaster@raasu.org> 20020924 -- Added id 2
				fPopup->insertItem(tr("List All Files"), 2);
				// <postmaster@raasu.org> 20020926 -- Added id 3
				fPopup->insertItem(tr("Get IP Address"), 3); 

				fPopupUser = uid;
				fPopup->popup(p);
			}
			it++;
		}
	}
}

void
WPrivateWindow::PopupActivated(int id)
{
	// <postmaster@raasu.org> 20020924 -- Add id detection
	WUserIter it = fNet->Users().find(fPopupUser);
	if (it != fNet->Users().end())
	{
		if (id == 1) 
		{
			RemUser((*it).second);
		}
		else if (id == 2) 
		{
			QString qPattern = "*@";
			qPattern += (*it).second()->GetUserID();
			WinShareWindow::LaunchSearch(qPattern);
		} 
		else if (id == 3) 
		{
			QString user = FixStringStr((*it).second()->GetUserName());
			QString host = (*it).second()->GetUserHostName();
			QString qTemp = WFormat::UserIPAddress(user, host); // <postmaster@raasu.org> 20021112
			PrintSystem(qTemp);
		}
	}
}

void
WPrivateWindow::UpdateUserList()
{
	fLock.lock();
	WUserIter it = fUsers.begin();
	while ( it != fUsers.end())
	{
		(*it).second()->AddToListView(fPrivateUsers);
		it++;
	}
	fLock.unlock();
}

void
WPrivateWindow::CheckEmpty()
{
	if (fUsers.empty())
	{
		switch (Settings()->GetEmptyWindows())
		{
		case 0: break;
		case 1:
			{
				if (QMessageBox::information(this, tr( "Private Chat" ), 
					tr( "There are no longer any users in this private chat window. Close window?"),
					tr( "Yes" ), tr( "No" )) == 0)	
					// 0 is the index of "yes"
				{
					done(QDialog::Accepted);
				}
				break;
			}
		case 2:
			{
				done(QDialog::Accepted);
				break;
			}
		}
	}
}

QWidget *
WPrivateWindow::Window()
{
	return this;
}

void
WPrivateWindow::LogString(const QString & text)
{
	fLog.LogString(text);
}

void
WPrivateWindow::LogString(const char *text)
{
	fLog.LogString(text);
}
