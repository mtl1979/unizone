#ifdef _WIN32
#include <windows.h>
#endif

#include <qapplication.h>
#include <qmessagebox.h>
#include <QCustomEvent>
#include <QResizeEvent>
#include <Q3ValueList>
#include <Q3PopupMenu>

#include "privatewindowimpl.h"
#include "privatewindow.h"
#include "gotourl.h"
#include "formatting.h"
#include "chatevent.h"
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

/* 
 *  Constructs a privatewindow which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
WPrivateWindow::WPrivateWindow(QObject * owner, NetClient * net, QWidget* parent,  const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl),
	ChatWindow(PrivateType)
{
	fOwner = owner;
	fNet = net;
	fEncrypted = false;

	Ui_WPrivateWindowBase *ui = new Ui_WPrivateWindowBase();
	ui->setupUi(this);

	if ( !name ) 
		setName( "WPrivateWindow" );
	// start GUI
	fSplit = new QSplitter(this);
	Q_CHECK_PTR(fSplit);

	// setup chat part
	fSplitChat = new QSplitter(fSplit);
	Q_CHECK_PTR(fSplitChat);

	fChatText = new WHTMLView(fSplitChat);
	Q_CHECK_PTR(fChatText);
	// we still want to autocomplete ALL names, not just
	// the one's with the ppl we talk to
	fInputText = new WChatText(this, fSplitChat);
	Q_CHECK_PTR(fInputText);

	// user list
	fPrivateUsers = new Q3ListView(fSplit);
	Q_CHECK_PTR(fPrivateUsers);

	InitUserList(fPrivateUsers);

	Q3ValueList<int> splitList;
	splitList.append(4);
	splitList.append(1);

	fSplit->setSizes(splitList);
	fSplitChat->setSizes(splitList);
	fSplitChat->setOrientation(Qt::Vertical);

	// create popup menu
	fPopup = new Q3PopupMenu(this);	// have it deleted on destruction of window
	Q_CHECK_PTR(fPopup);

	connect(fPopup, SIGNAL(activated(int)), this, SLOT(PopupActivated(int)));
	connect(fPrivateUsers, SIGNAL(rightButtonClicked(Q3ListViewItem *, const QPoint &, int)),
			this, SLOT(RightButtonClicked(Q3ListViewItem *, const QPoint &, int)));

	connect(fNet, SIGNAL(UserDisconnected(const WUserRef &)), 
			this, SLOT(UserDisconnected(const WUserRef &)));
	connect(fNet, SIGNAL(DisconnectedFromServer()), 
			this, SLOT(DisconnectedFromServer()));
	connect(fChatText, SIGNAL(URLClicked(const QString &)), 
			this, SLOT(URLClicked(const QString &)));
	connect(fInputText, SIGNAL(TabPressed(const QString &)), 
			this, SLOT(TabPressed(const QString &)));

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
	fLock.Lock();
	WUserIter it = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
	while ( it.HasMoreValues() )
	{
		WUserRef uref;
		it.GetNextValue(uref);
		uref()->RemoveFromListView(fPrivateUsers);
	}
	fLock.Unlock();
	WPWEvent *closed = new WPWEvent(WPWEvent::Closed);
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
	fUsers.Clear();
	if (Settings()->GetError())
		PrintError(tr("Disconnected from server."));
}

void
WPrivateWindow::UserDisconnected(const WUserRef & uref)
{
	bool ok;
	uint32 uid = uref()->GetUserID().toULong(&ok);
	if (ok)
	{
		QString name = uref()->GetUserName();
		if (fUsers.ContainsKey(uid))
		{
			if (Settings()->GetUserEvents())
			{
				QString msg = WFormat::UserDisconnected(uref()->GetUserID(), FixString(name)); 
				PrintSystem(msg);
			}
			uref()->RemoveFromListView(fPrivateUsers);
			fUsers.Remove(uid);
			
			CheckEmpty();
		}
	}
}

void
WPrivateWindow::URLClicked(const QString & url)
{
	if (url != QString::null)
	{
		QString surl;
		// <postmaster@raasu.org> 20021021 -- Use lower() to eliminate not matching because of mixed casing
		if (url.startsWith("beshare:", false) || 
			url.startsWith("share:", false))
		{
			surl = url.mid(url.find(":") + 1);
			WinShareWindow::LaunchSearch(surl);
		}
		else if (url.startsWith("ttp://", false))	// <postmaster@raasu.org> 20030911
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
	if (Settings()->GetPrivate())
	{
		bool ok;
		uint32 uid = fromsid.toULong(&ok);
		if (ok)
		{
			WUserRef uref;
			
			if (fUsers.GetValue(uid, uref) == B_NO_ERROR)
			{
				QString name = FixString(uref()->GetUserName());
				QString s;
				if ( IsAction(message, name) ) // simulate action?
				{
					s = WFormat::Action(FormatNameSaid(message));
				}
				else
				{
					s = WFormat::ReceivePrivMsg(fromsid, name, FormatNameSaid(message));
				}
				PrintText(s);
				if (Settings()->GetSounds())
					beep();
				// <postmaster@raasu.org> 20021021 -- Fix Window Flashing on older API's
#ifdef WIN32
				// flash away!
				if (!this->isActiveWindow() && (Settings()->GetFlash() & WSettings::FlashPriv))	// got the handle... AND not active? AND user wants us to flash
				{
					WFlashWindow(winId()); // flash
				}
#endif // WIN32
			}
		}
	}
}


void
WPrivateWindow::AddUser(const WUserRef & user)
{
	fLock.Lock();
	bool ok;
	uint32 uid = user()->GetUserID().toULong(&ok);
	if (ok)
	{
		if (!fUsers.ContainsKey(uid))
		{
			fUsers.Put(uid, user);
			user()->AddToListView(fPrivateUsers);
		}
	}
	fLock.Unlock();
}

bool
WPrivateWindow::RemUser(const WUserRef & user)
{
	fLock.Lock();
	bool ok;
	uint32 uid = user()->GetUserID().toULong(&ok);
	if (ok)
	{
		if (fUsers.ContainsKey(uid))
		{
			user()->RemoveFromListView(fPrivateUsers);
			fUsers.Remove(uid);
			fLock.Unlock();
			return true;
		}
	}
	fLock.Unlock();
	return false;
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
WPrivateWindow::customEvent(QEvent * event)
{
	PRINT("WPrivateWindow::customEvent\n");
	switch ((int) event->type())
	{
		case WChatEvent::ChatTextType:
		{
			WChatEvent *wce = dynamic_cast<WChatEvent *>(event);
			if (wce)
			{
				PutChatText(wce->Sender(), wce->Text());
			}
			return;
		}
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
					bool rem = CompareCommand(stxt, "/removeuser");

					QString targetStr, restOfString;
					WUserSearchMap sendTo;
					QString qTemp = GetParameterString(wte->Text());

					if (WinShareWindow::ParseUserTargets(qTemp, sendTo, targetStr, restOfString, fNet))
					{
						// got some users
						WUserRef user;
						if (sendTo.IsEmpty())
						{
							if (Settings()->GetError())
								PrintError( tr( "User(s) not found!" ) );
						}
						else
						{
							for (unsigned int qi = 0; qi < sendTo.GetNumItems(); qi++)
							{
								user = sendTo[qi].user;
								QString sid = user()->GetUserID();

								if (rem)	
								{
									bool ok;
									uint32 uid = sid.toULong(&ok);
									if (ok)
									{
										if (fUsers.ContainsKey(uid))
										{
											user()->RemoveFromListView(fPrivateUsers);
											if (Settings()->GetUserEvents())
											{
												PrintSystem(WFormat::PrivateRemoved(user()->GetUserID(), FixString(user()->GetUserName())));
											}
											fUsers.Remove(uid);
										}
									}
								}
								else // add a new user
								{
									// see if the user is already in the list
									bool ok;
									uint32 id = sid.toULong(&ok); 
									if ( ok && fUsers.ContainsKey(id) )
									{
										WUserRef found;
										fUsers.Get(id, found);
										if (Settings()->GetUserEvents())
											PrintError(tr("User #%1 (a.k.a %2) is already in this private window!").arg(sid).arg(user()->GetUserName()));

									}
									else	// user not yet in list? 
									{
										WUserIter it = fUsers.GetIterator(HTIT_FLAG_NOREGISTER);
										bool err = false;
										while (it.HasMoreValues())
										{
											WUserRef ref;
											it.GetNextValue(ref);
											if (ref()->IsBot() != user()->IsBot())
											{
												if (Settings()->GetError())
												{
													if (ref()->IsBot())
														PrintError(WFormat::PrivateIsBot(ref()->GetUserID(), FixString(ref()->GetUserName())));
													else
														PrintError(WFormat::PrivateIsBot(sid, FixString(user()->GetUserName())));
												}
												err = true;
												break;
											}
										}
										if (!err)		// No error -> add ;)	
											AddUser(user);	// the EASY way :)
									}
								}
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
					WString wmessage(message);
					PRINT("\t\t%S\n", wmessage.getBuffer());
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
					WString wmessage(message);
					PRINT("\t\t%S\n", wmessage.getBuffer());
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
					Clear();	// empty the text
				}
				else if (CompareCommand(stxt, "/encryption"))
				{
					QString qtext = GetParameterString(stxt);
					if (qtext == "on")
					{
						PrintSystem(tr("Encryption enabled."));
						fEncrypted = true;
					}
					else if (qtext == "off")
					{
						fEncrypted = false;
						PrintSystem(tr("Encryption disabled."));
					}
					else
					{
						PrintSystem(tr("Encryption is %1.").arg(fEncrypted ? "enabled" : "disabled"));
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

		case WPWEvent::Created:
		{
			show();
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
	fLog.Create(WLog::LogPrivate);	// create a private chat log
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
WPrivateWindow::RightButtonClicked(Q3ListViewItem * i, const QPoint & p, int /* c */)
{
	// empty menu
	while (fPopup->count() > 0)
		fPopup->removeItemAt(0);
	if (i)
	{
		QString uid = i->text(1);		// session ID
		bool ok;
		uint32 sid = uid.toULong(&ok);
		WUserMap & umap = fNet->Users();
		if (umap.ContainsKey(sid))
		{
			WUserRef uref;
			umap.Get(sid, uref);
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
	}
}

void
WPrivateWindow::PopupActivated(int id)
{
	// <postmaster@raasu.org> 20020924 -- Add id detection
	WUserRef uref = fNet->FindUser(fPopupUser);
	if (uref())
	{
		if (id == 1) 
		{
			RemUser(uref);
		}
		else if (id == 2) 
		{
			QString qPattern = "*@";
			qPattern += uref()->GetUserID();
			WinShareWindow::LaunchSearch(qPattern);
		} 
		else if (id == 3) 
		{
			QString qTemp = WFormat::UserIPAddress(FixString(uref()->GetUserName()), uref()->GetUserHostName()); // <postmaster@raasu.org> 20021112
			PrintSystem(qTemp);
		}
	}
}

void
WPrivateWindow::CheckEmpty()
{
	if (fUsers.IsEmpty())
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
	fLog.LogString(text, true);
}

void
WPrivateWindow::LogString(const char *text)
{
	fLog.LogString(text, true);
}
