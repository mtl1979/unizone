#ifdef WIN32
#include <windows.h>
#pragma warning(disable: 4786)
#endif

#include "privatewindowimpl.h"
#include "gotourl.h"
#include "formatting.h"
#include "textevent.h"
#include "global.h"
#include "settings.h"
#include "util/String.h"
#include "lang.h"				// <postmaster@raasu.org> 20020924
#include "platform.h"
#include "wpwevent.h"

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
    : WPrivateWindowBase(parent, name, modal, fl), fOwner(owner), fNet(net)
{
	if ( !name ) 
		setName( "WPrivateWindow" );
	// start GUI
	fSplit = new QSplitter(this);
	CHECK_PTR(fSplit);

	// setup chat part
	fSplitChat = new QSplitter(fSplit);
	CHECK_PTR(fSplitChat);

	fText = new WHTMLView(fSplitChat);
	CHECK_PTR(fText);
	// we still want to autocomplete ALL names, not just
	// the one's with the ppl we talk to
	fChat = new WChatText(this, fSplitChat);
	CHECK_PTR(fText);

	// user list
	fPrivateUsers = new QListView(fSplit);
	fPrivateUsers->addColumn(tr(MSG_NL_NAME));
	fPrivateUsers->addColumn(tr("ID"));
	fPrivateUsers->addColumn(tr(MSG_NL_STATUS));
	fPrivateUsers->addColumn(tr(MSG_NL_FILES));
	fPrivateUsers->addColumn(tr(MSG_NL_CONNECTION));
	fPrivateUsers->addColumn(tr(MSG_NL_LOAD));
	fPrivateUsers->addColumn(tr(MSG_NL_CLIENT));		// as of now... winshare specific, WinShare pings all the users and parses the string for client info

	fPrivateUsers->setColumnAlignment(1, AlignRight); // <postmaster@raasu.org> 20021005
	fPrivateUsers->setColumnAlignment(3, AlignRight); // <postmaster@raasu.org> 20021005
	fPrivateUsers->setColumnAlignment(5, AlignRight); // <postmaster@raasu.org> 20021005

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

	connect(fNet, SIGNAL(UserDisconnected(QString, QString)), this,
			SLOT(UserDisconnected(QString, QString)));
	connect(fNet, SIGNAL(DisconnectedFromServer()), this,
			SLOT(DisconnectedFromServer()));
	connect(fText, SIGNAL(URLClicked()), this, SLOT(URLClicked()));
	connect(fText, SIGNAL(highlighted(const QString&)), this,
			SLOT(URLSelected(const QString&)));
	connect(fChat, SIGNAL(TabPressed(QString)), this, SLOT(TabPressed(QString)));
#ifndef WIN32
	connect(fText, SIGNAL(GotShown(const QString &)), this, SLOT(GotShown(const QString &)));
#endif

#ifdef WIN32
	// <postmaster@raasu.org> 20021021 -- Use Unicode macro L"..."
	fWinHandle = FindWindow(NULL, L"[Freeware] - Private");
	// <postmaster@raasu.org> 20020925
	if (fWinHandle)
	{
		PRINT("Got Handle for private window!\n");
	}
#endif

	if (gWin->fSettings->GetLogging())
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
	if (gWin->fSettings->GetError())
		PrintError(tr(MSG_DISCONNECTED));
}

void
WPrivateWindow::UserDisconnected(QString sid, QString name)
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
		(*iter).second()->RemoveFromListView(fPrivateUsers);
		fUsers.erase(iter);

		if (fUsers.empty())
		{
			if (QMessageBox::information(this, "Private Chat", "There are no longer any users in this private chat window."
										 " Close window?", "Yes", "No") == 0)	// 0 is the index of "yes"
										 done(QDialog::Accepted);
		}
	}
}

void
WPrivateWindow::URLClicked()
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
WPrivateWindow::URLSelected(const QString & href)
{
	fURL = href;
}

void
WPrivateWindow::UpdateView()
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
WPrivateWindow::PrintText(const QString & str)
{
	QString output("");

	// Check for timestamp
	if (gWin->fSettings->GetTimeStamps())
		output = WinShareWindow::GetTimeStamp();
	output += str;

	if (gWin->fSettings->GetLogging())
		fLog.LogString(output.latin1());
	CheckScrollState();
	fText->append(output);
	UpdateView();
}

void
WPrivateWindow::PrintSystem(const QString & msg)
{
	QString s = WFormat::SystemText().arg(WColors::System).arg(gWin->fSettings->GetFontSize()); // <postmaster@raasu.org> 20021127 -- Wrong order!!
	s += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(ParseChatText((QString &)msg));
	PrintText(s);
}

void
WPrivateWindow::PutChatText(QString fromsid, QString msg)
{
	WUserIter it = fUsers.find(fromsid);
	FixString(msg);

	if (it != fUsers.end())
	{
		if (gWin->fSettings->GetPrivate())
		{
			QString name = (*it).second()->GetUserName();
			FixString(name);
			QString s;
			if (msg.startsWith(name + " ") || msg.startsWith(name + "'s ")) // simulate action?
			{
				s = WFormat::Action().arg(WColors::Action).arg( gWin->fSettings->GetFontSize() );
				s += WFormat::Text.arg(WColors::Text).arg(gWin->fSettings->GetFontSize()).arg(msg);
			}
			else
			{
				s = WFormat::ReceivePrivMsg.arg(WColors::RemoteName).arg(
				gWin->fSettings->GetFontSize()).arg(fromsid).arg(name).arg(WColors::PrivText).arg(
				gWin->fSettings->GetFontSize()).arg(msg);
			}
			PrintText(s);
			QApplication::beep();
			// <postmaster@raasu.org> 20021021 -- Fix Window Flashing on older API's
#ifdef WIN32
			// flash away!
			if (fWinHandle && !this->isActiveWindow() && (gWin->fSettings->GetFlash() & WSettings::FlashPriv))	// got the handle... AND not active? AND user wants us to flash
			{
				WFlashWindow(fWinHandle); // flash
			}
#endif // WIN32
		}
	}
}

void
WPrivateWindow::AddUser(WUserRef & user)
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
WPrivateWindow::RemUser(WUserRef & user)
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
WPrivateWindow::TabPressed(QString str)
{
	PRINT("WPrivateWindow::Tab\n");
	WPWEvent *e = new WPWEvent(WPWEvent::TabComplete, fChat->text());
	if (e)
	{
		e->SetSendTo(this);
		QApplication::postEvent(fOwner, e);
	}
}

void
WPrivateWindow::customEvent(QCustomEvent * event)
{
	PRINT("WPrivateWindow::Event\n");
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
				if (wte->Text().lower().startsWith("/adduser ") ||
					wte->Text().lower().startsWith("/removeuser "))
				{
					bool rem = wte->Text().lower().startsWith("/adduser ") ? false : true;

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
							if (gWin->fSettings->GetError())
								PrintError("User(s) not found!\n");
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
									if (gWin->fSettings->GetError())
										PrintError(tr("User #%1 (a.k.a. %2) is a bot!\n").arg(sid).arg(user()->GetUserName()));
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
											if (gWin->fSettings->GetUserEvents())
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
										if (gWin->fSettings->GetUserEvents())
										{
											PrintSystem(tr("User #%1 (a.k.a. %2) was removed from the private chat window.").
														arg((*foundIt).second()->GetUserID()).arg((*foundIt).second()->GetUserName()));
										}
										fUsers.erase(foundIt);
									}
								}
								iter++;
							}
						}
						if (rem)	// check to see whether we have an empty list
						{
							if (fUsers.empty())
							{
								if (QMessageBox::information(this, "Private Chat", "There are no longer any users in this private chat window."
															 " Close window?", "Yes", "No") == 0)	// 0 is the index of "yes"
									done(QDialog::Accepted);
							}
						}
					}
				}
				else if (wte->Text().lower().startsWith("/action's ") ||
						wte->Text().lower().startsWith("/me's "))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += "'s ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					PRINT("\t\t%s\n", message.latin1());
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, message);
					if (e)
					{
						e->SetSendTo(this);
						QApplication::postEvent(fOwner, e);
					}
				}
				else if (wte->Text().lower().startsWith("/action ") ||
						wte->Text().lower().startsWith("/me "))
				{
					QString stxt(wte->Text());
					QString message = gWin->GetUserName();
					message += " ";
					message += GetParameterString(stxt); // <postmaster@raasu.org> 20021021 -- Use Special Function to check validity
					PRINT("\t\t%s\n", message.latin1());
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, message);
					if (e)
					{
						e->SetSendTo(this);
						QApplication::postEvent(fOwner, e);
					}
				}
				else if (wte->Text().lower().startsWith("/clear"))
				{
					fText->setText("");	// empty the text
				}
				else
				{
					WPWEvent *e = new WPWEvent(WPWEvent::TextEvent, fUsers, wte->Text());
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
WPrivateWindow::PrintError(const QString & error)
{
	if (gWin->fSettings->GetError())
	{
		QString e = WFormat::Error().arg(WColors::Error).arg(gWin->fSettings->GetFontSize());
		e += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(gWin->fSettings->GetFontSize()).arg(error);
		PrintText(e);
	}
}

void
WPrivateWindow::GotShown(const QString & txt)
{
	fText->setText(WinShareWindow::ParseForShown(txt));
}

void
WPrivateWindow::CheckScrollState()
{
	QScrollBar * scroll = fText->verticalScrollBar();
	if (scroll->value() >= scroll->maxValue())
		fScrollDown = true;
	else
		fScrollDown = false;
}

void
WPrivateWindow::StartLogging()
{
	fLog.Create(true);	// create a private chat log
	if (!fLog.InitCheck())
	{
		if (gWin->fSettings->GetError())
			PrintError("Failed to create private log.");
	}
}

void
WPrivateWindow::StopLogging()
{
	fLog.Close();
}

void
WPrivateWindow::RightButtonClicked(QListViewItem * i, const QPoint & p, int c)
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
				QString txt = tr(MSG_REMOVE);
				// <postmaster@raasu.org> 20020924 -- Added ',1'
				fPopup->insertItem(txt,1);
				// <postmaster@raasu.org> 20020924 -- Added id 2
				txt = tr(MSG_NL_LISTALLFILES);
				fPopup->insertItem(txt,2);
				// <postmaster@raasu.org> 20020926 -- Added id 3
				txt = tr(MSG_NL_GETIPADDR);
				fPopup->insertItem(txt,3); 

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
			QString qTemp = tr(MSG_USERHOST).arg(FixStringStr((*it).second()->GetUserName())).arg((*it).second()->GetUserHostName()).arg(WColors::RemoteName); // <postmaster@raasu.org> 20021112
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

