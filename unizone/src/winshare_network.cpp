// An extension of the main WinShare class
// Contains all the networking related methods

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

#include "winsharewindow.h"
#include "settings.h"
#include "debugimpl.h"
#include "formatting.h"
#include "textevent.h"
#include "version.h"
#include "lang.h"				// <postmaster@raasu.org> 20020924
#include "platform.h"			// <postmaster@raasu.org> 20021021
								// For GetParameterString that will split user command parameters from string 
								// containing both command and parameters, 
								// can handle situation when no parameters have been passed
#include "gotourl.h"			// <postmaster@raasu.org> 20021116 -- for /shell
#include "tokenizer.h"			// <postmaster@raasu.org> 20030902 -- for !remote
#include <time.h>				//                                 -- for /time
#include "util/TimeUtilityFunctions.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "util/StringTokenizer.h"

#include <qapplication.h>

void
WinShareWindow::SendChatText(WTextEvent * e, bool * reply)
{
	if (e)	// incase the dynamic cast fails
	{
		PRINT("WinShareWindow: Received Valid WTextEvent\n");
		QString sendText = e->Text();

		sendText = sendText.stripWhiteSpace();
		if (CompareCommand(sendText, "/ping"))
		{
			SendPingOrMsg(sendText, true);
		}
		else if (CompareCommand(sendText, "/scan"))
		{
			if (fSettings->GetSharingEnabled())
			{
				if (fNetClient->IsInternalThreadRunning())	// are we connected?
				{
					if (fSettings->GetInfo())
						PrintSystem(MSG_RESCAN);
					if (!fFileScanThread->IsRunning())
						fFileScanThread->start();
					else
						PrintError(MSG_SCAN_ERR1,false);
				}
				else
				{
					if (fSettings->GetError())
						PrintError(MSG_NOTCONNECTED);
				}
			}
			else
			{
				if (fSettings->GetError())
					PrintError(MSG_NOTSHARING);
			}
		}
		else if (CompareCommand(sendText, "/msg"))
		{
			SendPingOrMsg(sendText, false, reply);
			if (reply && *reply)		// is reply wanted? and should it be sent?
				e->SetText(sendText);	// modified by SendPingOrMsg();
		}
		else if (CompareCommand(sendText, "/nick"))
		{
			// user name change
			QString name = GetParameterString(sendText);
			// <postmaster@raasu.org> 20021001
			if (name.length() > 0)
			{
				NameChanged(name);
				// see if it exists in the list yet...
				for (int i = 0; i < fUserList->count(); i++)
				{
					if ( name == fUserList->text(i) )	// found match?
					{
						fUserList->setCurrentItem(i);
						return;
					}
				}
				// otherwise, insert
				fUserList->insertItem(name, 0);
				fUserList->setCurrentItem(0);
			}
			else
			{
				if (fSettings->GetError())
					PrintError(MSG_NONICK);
			}
		}
		else if (	
					(CompareCommand(sendText, "/me's")) || 
					(CompareCommand(sendText, "/action's")) 
				)
		{
			QString msg = GetParameterString(sendText);
			
			if (msg.length() > 0)
			{
				QString send = "/me's ";
				send += msg;

				fNetClient->SendChatText("*", send);
				if (fSettings->GetChat())
					Action(GetUserName() + "'s", msg);
            }
			else
			{
				if (fSettings->GetError())
					PrintError(MSG_NOMESSAGE);
			}
		}
		else if (	
					(CompareCommand(sendText, "/me")) || 
					(CompareCommand(sendText, "/action")) 
				)
		{
			QString msg = GetParameterString(sendText);

			if (msg.length() > 0)
			{
				QString send = "/me ";	// we don't want /action sent
				send += msg;

				fNetClient->SendChatText("*", send);
				if (fSettings->GetChat())
					Action(GetUserName(), msg);
			}
			else
			{
				if (fSettings->GetError())
					PrintError(MSG_NOMESSAGE);
			}
		}
		else if (sendText.left(2) == "//")	// used so that / commands can be printed
		{
			sendText.replace(0, 2, "/");
			SendChatText("*", sendText);
		}
		else if (CompareCommand(sendText, "/help"))
		{
			if (fSettings->GetInfo())
				ShowHelp();
		}
		else if (CompareCommand(sendText, "/clearline"))
		{
			fInputText->ClearBuffer();
			pLock.lock();
			for (WPrivIter it = fPrivateWindows.begin(); it != fPrivateWindows.end(); it++)
				(*it).first->ClearBuffer();
			pLock.unlock();
		}
		else if (CompareCommand(sendText, "/quit"))
		{
			Exit();
		}
		else if (CompareCommand(sendText, "/status"))
		{
			// change our status
			QString newStatus = GetParameterString(sendText);

			if (newStatus.length() > 0)
			{
				QString ns = newStatus.stripWhiteSpace();
				bool found = false;

				int i;	// this way we know which one was found

				for (i = 0; i < fStatusList->count(); i++)
				{
					QString status = fStatusList->text(i).stripWhiteSpace();
					if (status.lower() == ns.lower())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					fStatusList->insertItem(ns, 0); // <postmaster@raasu.org> 20021005
					StatusChanged(ns);
				}
				else
				{
					fStatusList->setCurrentItem(i);
					StatusChanged(fStatusList->currentText());
				}
			}
		}
		else if (CompareCommand(sendText, "/serverinfo"))
		{
			fNetClient->SendMessageToSessions(MessageRef(new Message(PR_COMMAND_GETPARAMETERS), NULL));
		}
		else if (CompareCommand(sendText, "/server"))
		{
			QString server = GetParameterString(sendText);

			if (server.length() > 0)
				{
					QString serverStr = server.stripWhiteSpace();

					GotUpdateCmd("addserver", server);	// the easy way to add the server to the list...
														// this will check for duplicates
					// now find our server
					for (int i = 0; i < fServerList->count(); i++)
					{
						QString slist = fServerList->text(i).stripWhiteSpace();
						if (slist.lower() == serverStr.lower())
						{
							// found our server
							fServerList->setCurrentItem(i);
							ServerChanged(fServerList->currentText());
						}
					}
				}
			
		}
		else if (CompareCommand(sendText, "/connect"))
		{
			Connect();
		}
		else if (CompareCommand(sendText, "/disconnect"))
		{
			Disconnect();
		}
		else if (CompareCommand(sendText, "/awaymsg"))
		{
			QString msg = GetParameterString(sendText);

			if (msg.length()>0)
			{
				fAwayMsg = msg;
				if (fSettings->GetInfo())
					PrintSystem(tr(MSG_AWAYMESSAGE).arg(fAwayMsg));
			}
		}
		else if (CompareCommand(sendText, "/away"))
		{
			fAutoAway->stop();
			fAway = true;
			SetStatus(fAwayMsg);
		}
		else if (CompareCommand(sendText, "/heremsg"))
		{
			QString msg = GetParameterString(sendText);

			if (msg.length()>0)
			{
				fHereMsg = msg;
				if (fSettings->GetInfo())
					PrintSystem(tr(MSG_HEREMESSAGE).arg(fHereMsg));
			}
		}
		else if (CompareCommand(sendText, "/clear"))
		{
			fChatText->setText("");	// empty the text
		}
		else if (CompareCommand(sendText, "/watch"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
				SetWatchPattern(users);
			else
				SetWatchPattern("");	// clear watch pattern
		}
#ifdef WIN32
		// <postmaster@raasu.org> 20021026
		else if (CompareCommand(sendText, "/uptime"))
		{
			PrintSystem( tr("Uptime: %1").arg(GetUptimeString()) );
		}
		// <postmaster@raasu.org> 20021116
		else if (CompareCommand(sendText, "/buptime"))
		{
			QString text = tr("Uptime: %1").arg(GetUptimeString());
			if (fNetClient->IsConnected())
				fNetClient->SendChatText("*", text);
			PrintSystem(text);
		}
#endif
		// <postmaster@raasu.org> 20020929
		else if (CompareCommand(sendText, "/users"))
		{
			int iUsers = fUsers->childCount() + 1; // <postmaster@raasu.org> 20021005 -- I think it's 1 for current user that needs to be added to get the total count
			PrintSystem(tr(MSG_NUM_USERS).arg(iUsers));
		}
		else if (CompareCommand(sendText, "/priv"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
					LaunchPrivate(users);
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/kick"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_KICK), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/kickips"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_KICK), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) users.utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/addbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_ADDBANS), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/addipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_ADDBANS), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) users.utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/rembans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_REMOVEBANS), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/remipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_REMOVEBANS), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) users.utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/require"))
		{
			QString mask = GetParameterString(sendText);

			if (mask.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_ADDREQUIRES), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) mask.utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/unrequire"))
		{
			QString mask = GetParameterString(sendText);

			if (mask.length() > 0)
			{
				MessageRef mrf(new Message(PR_COMMAND_REMOVEREQUIRES), NULL);
				mrf()->AddString(PR_NAME_KEYS, (const char *) mask.utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		// <postmaster@raasu.org> 20021116
#if defined(WIN32)
		else if (CompareCommand(sendText, "/shell"))
		{
			QString command = GetParameterString(sendText);

			if (command.length() > 0)
			{
				GotoURL(command);
			}
		}
#endif
		else if (CompareCommand(sendText, "/time"))
		{
			QString command = GetParameterString(sendText).lower();
			String lt;
			time_t currentTime = time(NULL);
			tm * myTime;
			char zone[64];

			if (command != "gmt")
				myTime = localtime(&currentTime);
			else
				myTime = gmtime(&currentTime);

			lt = asctime(myTime);
			lt = lt.Substring(0, "\n");

			if (command == "gmt")
				strcpy(zone, "GMT");
			else if (myTime->tm_isdst != 1) 
				strcpy(zone, tzname[0]);
			else 
				strcpy(zone, tzname[1]);
			
			PrintSystem(tr("Current time: %1 %2").arg(lt.Cstr()).arg(zone),false);
		}
		else if (CompareCommand(sendText,"/btime"))
		{
			QString command = GetParameterString(sendText).lower();
			String lt;
			time_t currentTime = time(NULL);
			tm * myTime;
			char zone[64];

			if (command != "gmt")
				myTime = localtime(&currentTime);
			else
				myTime = gmtime(&currentTime);

			lt = asctime(myTime);
			lt = lt.Substring(0, "\n");

			if (command == "gmt")
				strcpy(zone, "GMT");
			else if (myTime->tm_isdst != 1) 
				strcpy(zone, tzname[0]);
			else 
				strcpy(zone, tzname[1]);

			QString text = tr("Current time: %1 %2").arg(lt.Cstr()).arg(zone);

			if (fNetClient->IsConnected())
				fNetClient->SendChatText("*", text);
			PrintSystem(text);
		}
		else if (CompareCommand(sendText, "/ignore"))
		{
			fIgnore = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fIgnore == "")
					PrintSystem("Ignore pattern cleared.");
				else
					PrintSystem(tr("Ignore pattern set to: %1").arg(fIgnore));
			}
		}
		else if (CompareCommand(sendText, "/addignore"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = Ignore(user);

			if (bSuccess)
				PrintSystem("Ignore pattern updated.");
			else if (fSettings->GetError())
				PrintError("Error updating ignore pattern!");
		}
		else if (CompareCommand(sendText, "/unignore"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnIgnore(user);

			if (bSuccess)
				PrintSystem("Ignore pattern updated.");
			else if (fSettings->GetError())
				PrintError("Error updating ignore pattern!");
		}
		else if (CompareCommand(sendText, "/blacklist"))
		{
			fBlackList = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fBlackList == "")
					PrintSystem("Blacklist pattern cleared.");
				else
					PrintSystem(tr("Blacklist pattern set to: %1").arg(fBlackList));
			}
		}
		else if (CompareCommand(sendText, "/addblacklist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = BlackList(user);

			if (bSuccess)
				PrintSystem("Blacklist pattern updated.");
			else if (fSettings->GetError())
				PrintError("Error updating blacklist pattern!");
		}
		else if (CompareCommand(sendText, "/unblacklist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnBlackList(user);

			if (bSuccess)
				PrintSystem("Blacklist pattern updated.");
			else if (fSettings->GetError())
				PrintError("Error updating blacklist pattern!");
		}
		else if (CompareCommand(sendText, "/chkuser"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fUserList->count()))
					PrintSystem( tr("User %1: %2").arg(u).arg(fUserList->text(u)) );
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/remuser"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fUserList->count()))
				{
					PrintSystem(tr("User %1 (%2) removed").arg(u).arg(fUserList->text(u)));
					fUserList->removeItem(u);
				}
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/chkstatus"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fStatusList->count()))
					PrintSystem( tr("Status %1: %2").arg(u).arg(fStatusList->text(u)) );
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NO_INDEX);
		}
		else if (CompareCommand(sendText, "/remstatus"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fStatusList->count()))
				{
					PrintSystem(tr("Status %1 (%2) removed").arg(u).arg(fStatusList->text(u)));
					fStatusList->removeItem(u);
				}
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NOUSERS);
		}
		else if (CompareCommand(sendText, "/chkserver"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fServerList->count()))
					PrintSystem( tr("Server %1: %2").arg(u).arg(fServerList->text(u)) );
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NO_INDEX);
		}
		else if (CompareCommand(sendText, "/remserver"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.length() > 0)
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fServerList->count()))
				{
					PrintSystem(tr("Server %1 (%2) removed").arg(u).arg(fServerList->text(u)));
					fServerList->removeItem(u);
				}
				else if (fSettings->GetError())
					PrintError(MSG_INVALID_INDEX);
			}
			else if (fSettings->GetError())
				PrintError(MSG_NO_INDEX);
		}
		else if (CompareCommand(sendText, "/save"))
		{
			PrintSystem("Saving configuration...");
			SaveSettings();
			PrintSystem("Saved.");
		}
		
		else if (CompareCommand(sendText, "/dns"))
		{
			QString user = GetParameterString(sendText);
			QString uid = "";
			QString addr;
			WUserRef uref = FindUser(user);
			uint32 address = 0;
			String host;
			struct in_addr iaHost;     // Internet address structure
			LPHOSTENT lpHostEntry;     // Pointer to host entry structure
			
			if (uref() != NULL)
			{
				address = GetHostByName(uref()->GetUserHostName().latin1());
				addr = uref()->GetUserHostName();
				user = uref()->GetUserName();
				uid = uref()->GetUserID();
			}
			else
			{
				address = GetHostByName(user.latin1());
				addr = user;
			}

			if (address > 0)
			{
				host = Inet_NtoA(address);
				
				if (uid != "")
					PrintSystem( tr("Address info for user #%1 (%2):").arg(uid).arg(user), false);
				else
					PrintSystem( tr("Address info for %1:").arg(user), false);

				PrintSystem( tr("IP Address: %1").arg(host.Cstr()), false);
				iaHost.s_addr = inet_addr(host.Cstr());
				lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
				
				if (lpHostEntry != NULL)
				{
					PrintSystem( tr("Host Name: %1").arg(lpHostEntry->h_name), false);
				}

				if (uid == "")
				{
					// List all users from this ip
					START_OUTPUT();
					PrintSystem( "Connected users: ", true);

					WUserMap cmap;
					fNetClient->FindUsersByIP(cmap, host.Cstr());
					for (WUserIter it = cmap.begin(); it != cmap.end(); it++)
					{
						if ( (*it).second() )
						{
							PrintSystem( tr("#%1 - %2").arg( (*it).second()->GetUserID() ).arg( (*it).second()->GetUserName() ), true);
						}
					}
					END_OUTPUT();
				}
			}
			else
			{
				if (addr != user)
					PrintError(tr("No address info for %1 or %2").arg(user).arg(addr));
				else
					PrintError(tr("No address info for %1").arg(user));
			}
		}
		else if (CompareCommand(sendText, "/showstats"))
		{
			PrintSystem( tr("Uploaded %1, downloaded %2").arg(MakeSizeString(tx)).arg(MakeSizeString(rx)) );
		}
		else if (CompareCommand(sendText, "/showpatterns"))
		{
			PrintSystem( tr("Blacklist pattern: %1").arg(fBlackList) );
			PrintSystem( tr("Ignore pattern: %1").arg(fIgnore) );
			PrintSystem( tr("Watch pattern: %1").arg(fWatch) );
		}
		else if (CompareCommand(sendText, "/clearstats"))
		{
			tx = rx = 0;
			PrintSystem("Transfer statistics cleared.");
		}
		else if (CompareCommand(sendText, "/remote"))
		{
			QString p = GetParameterString(sendText);
			if (p == "")
				PrintSystem( tr("Remote password: %1").arg(fRemote) );
			else
			{
				fRemote = p;
				PrintSystem( tr("Remote password set to: %1").arg(p) );
			}	
		}
		// add more commands BEFORE this one

		else if (sendText.left(1) == "/")
		{
			// unknown command...
			if (reply)
			{
				*reply = true;
				// format an error string
				sendText = WFormat::Error.arg(WColors::Error).arg(fSettings->GetFontSize());
				sendText += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(fSettings->GetFontSize()).arg(MSG_UNKNOWNCMD);
				e->SetText(sendText);
			}
			else
			{
				if (fSettings->GetError())
					PrintError(MSG_UNKNOWNCMD);
			}
		}
		else
		{
			SendChatText("*", sendText);
		}
	}
}

void
WinShareWindow::SendChatText(QString sid, QString txt, WUserRef priv, bool * reply)
{
	fNetClient->SendChatText(sid, txt);
	if (sid != "*")	// not a global message?
	{
		if (priv())
		{
			// "reply" is handled here...
			if (!reply)	// no reply requested? print text out
			{
				if (fSettings->GetPrivate())	// do we take private messages?
				{
					QString chat;
					FixString(txt);
					PRINT("Appending to chat\n");
					if (txt.startsWith(FixStringStr(GetUserName()) + " ") || txt.startsWith(FixStringStr(GetUserName()) + "'s ")) // simulate action?
					{
						chat = WFormat::Action.arg(WColors::Action).arg( fSettings->GetFontSize() );
						chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(txt);
					}
					else
					{
						chat = WFormat::SendPrivMsg.arg(WColors::LocalName).arg(fSettings->GetFontSize()).arg(fNetClient->LocalSessionID()).arg(
									FixStringStr(GetUserName())).arg(FixStringStr(priv()->GetUserName()));
						chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(txt);
					}
					PRINT("Printing\n");
					PrintText(chat);
				}
			}
		}
	}
	else
	{
		if (fSettings->GetChat())
		{
			QString chat = WFormat::LocalName.arg(WColors::LocalName).arg(fSettings->GetFontSize()).arg(fNetClient->LocalSessionID()).arg(FixStringStr(GetUserName()));
			chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg( FixStringStr(txt) );
			PrintText(chat);
		}
	}
	// change away state
	if (fAway)
	{
		SetStatus(fHereMsg);
		fAway = false;
	}
	SetAutoAwayTimer();	// reinit away
}

void
WinShareWindow::SendPingOrMsg(QString & text, bool isping, bool * reply)
{
	String targetStr, restOfString;
	WUserSearchMap sendTo;
	QString sText;
	// <postmaster@raasu.org> 20021026
	sText = GetParameterString(text);

	if (ParseUserTargets(sText, sendTo, targetStr, restOfString, fNetClient))
	{
		WUserSearchIter iter = sendTo.begin();
		WUserRef user;

		if (sendTo.empty())
		{
			if (!isping && !reply)
			{
				if (fSettings->GetError())
					PrintError(MSG_USERSNOTFOUND);
			}
			else if (!isping && reply)
			{
				if (fSettings->GetError())
				{
					*reply = true;
					// this will put an error message in the private window
					QString error = WFormat::Error.arg(WColors::Error).arg(fSettings->GetFontSize());
					error += WFormat::ErrorMsg.arg(WColors::ErrorMsg).arg(fSettings->GetFontSize()).arg("User doesn't exist!\n");
					text = error;
				}
			}
		}
		else
		{
			QString qsendtext;
			while (iter != sendTo.end())
			{
				PRINT("Found user %s and rest of text %s\n", (*iter).second.user()->GetUserName().latin1(), (*iter).second.text.latin1());
				user = (*iter).second.user;

				QString sid = user()->GetUserID();
				QString sendText = (*iter).second.text;

				if (isping)
				{
					if (fSettings->GetInfo())
					{
						QString pingMsg = tr(MSG_PINGSENT).arg(sid).arg(FixStringStr(user()->GetUserName())).arg(WColors::RemoteName); // <postmaster@raasu.org> 20021112
						PrintSystem(pingMsg);
					}
					fNetClient->SendPing(sid);
				}
				else
				{
					// the reply only has an effect when used with /msg
					SendChatText(sid, sendText, user, reply);
				}
				iter++;
				if (iter == sendTo.end())
					qsendtext =  sendText;
			}
			if (!isping && reply)
			{
				if (fSettings->GetPrivate())
				{
					*reply = true;
					// format a wonderful string for our private window
					QString name = FixStringStr(GetUserName());
					qsendtext = FixStringStr(qsendtext);
					QString fmt;
					if (qsendtext.startsWith(name+" ") || qsendtext.startsWith(name + "'s ")) // simulate action?
					{
						fmt = WFormat::Action.arg(WColors::Action).arg(fSettings->GetFontSize());
					}
					else
					{
						fmt = WFormat::LocalName.arg(WColors::LocalName).arg(fSettings->GetFontSize()).arg(fNetClient->LocalSessionID()).arg(name);
					}
					fmt += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(qsendtext);
					text = fmt;
				}
			}
		}
	}
	else
	{
		if (fSettings->GetError())
			PrintError(MSG_NOUSERS);
	}
}

void
WinShareWindow::HandleSignal()
{
	MessageRef next;
	uint32 code;
	String sid;
	uint16 port;
	bool gotIncoming = false;

	//fNetClient->Lock();
	PRINT("SignalOwner\n");
	while (fNetClient->GetNextEventFromInternalThread(code, &next, &sid, &port) >= 0)
	{
		switch (code)
		{
			case MTT_EVENT_INCOMING_MESSAGE:
			{
				if (gotIncoming == false)
				{
					gotIncoming = true;
					fPrintOutput = false;
					PrintText("", true);	// beginning batch
				}
				PRINT("MTT_EVENT_INCOMING_MESSAGE\n");
				if (next())
				{
					switch (next()->what)
					{
						case PR_RESULT_PARAMETERS:
						{
							if (!fGotParams)
							{
								fNetClient->HandleParameters(next);
								fGotParams = true;
							}
							else	// a /serverinfo was sent
							{
								ServerParametersReceived(next());
							}
							break;
						}

						case PR_RESULT_DATAITEMS:
						{
							PRINT("PR_RESULT_DATAITEMS\n");
							fNetClient->HandleResultMessage(next);
							// add/remove all the users to the list view (if not there yet...)
							UpdateUserList();
							QCustomEvent * qce = new QCustomEvent(WinShareWindow::UpdatePrivateUsers);
							if (qce)
								QApplication::postEvent(this, qce);

							break;
						}

						case PR_RESULT_ERRORACCESSDENIED:
						{
							PRINT("PR_RESULT_ERRORACCESSDENIED\n");
							PrintError("Access Denied!!!");
							MessageRef subMsg;
							QString action = "do that to";

							String who;
							if (next()->FindMessage(PR_NAME_REJECTED_MESSAGE, subMsg) == B_NO_ERROR)
							{
								if (subMsg())
								{
									switch(subMsg()->what)
									{
										case PR_COMMAND_KICK:			action = "kick";		break;
										case PR_COMMAND_ADDBANS:		action = "ban";			break;
										case PR_COMMAND_REMOVEBANS:		action = "unban";		break;
										case PR_COMMAND_ADDREQUIRES:    action = "require";		break;
										case PR_COMMAND_REMOVEREQUIRES: action = "unrequire";	break;
									}
									if (subMsg()->FindString(PR_NAME_KEYS, who) == B_NO_ERROR)
									{
										QString qWho = QString::fromUtf8(who.Cstr());
										PrintError( tr("You are not allowed to %1 [%2]").arg(action).arg(qWho) );
									}
								}
							}
							break;
						}

						default:
						{
							PRINT("Handling message\n");
							HandleMessage(next());
							break;
						}
					}
				}
				break;
			}
	        case MTT_EVENT_SESSION_ACCEPTED:
			{
				PRINT("MTT_EVENT_SESSION_ACCEPTED\n");
				break;
			}
            case MTT_EVENT_SESSION_ATTACHED:
			{
				PRINT("MTT_EVENT_SESSION_ATTACHED\n");
				QCustomEvent *e = new QCustomEvent(NetClient::SessionAttached);
				if (e)
					QApplication::postEvent(this, e);
				break;
			}
            case MTT_EVENT_SESSION_CONNECTED:
			{
				PRINT("MTT_EVENT_SESSION_CONNECTED\n");
				QCustomEvent *e = new QCustomEvent(NetClient::SessionConnected);
				if (e)
					QApplication::postEvent(this, e);
				PRINT("Returning\n");
				break;
			}
            case MTT_EVENT_SESSION_DISCONNECTED:
			{
				PRINT("MTT_EVENT_SESSION_DISCONNECTED\n");
				QCustomEvent *e = new QCustomEvent(NetClient::Disconnected);
				if (e)
					QApplication::postEvent(this, e);
			}
            case MTT_EVENT_SESSION_DETACHED:
			{
				PRINT("MTT_EVENT_SESSION_DETACHED\n");
				QCustomEvent *e = new QCustomEvent(NetClient::Disconnected);
				QApplication::postEvent(this, e);
				break;
			}
            case MTT_EVENT_FACTORY_ATTACHED:
			{
				PRINT("MTT_EVENT_FACTORY_ATTACHED\n");
				break;
			}
            case MTT_EVENT_FACTORY_DETACHED:
			{
				PRINT("MTT_EVENT_FACTORY_DETACHED\n");
				break;
			}
            case MTT_EVENT_OUTPUT_QUEUES_DRAINED:
			{
				PRINT("MTT_EVENT_OUTPUT_QUEUES_DRAINED\n");
				break;
			}
            case MTT_EVENT_SERVER_EXITED:
			{
				PRINT("MTT_EVENT_SERVER_EXITED\n");
				// as you noticed... this message is sent by several MTT_EVENT_* events :)
				QCustomEvent *e = new QCustomEvent(NetClient::Disconnected);
				if (e)
					QApplication::postEvent(this, e);
				break;
			}
		}
	}
	if (gotIncoming)
	{
		fPrintOutput = true;
		PrintText("", false);
	}

	//fNetClient->Unlock();

	PRINT("SignalEvent: Update thread\n");
	// handle update thread
	if (fUpdateThread->IsInternalThreadRunning() && fSettings->GetCheckNewVersions())
	{
		PRINT("Checking update thread\n");
		while (fUpdateThread->GetNextEventFromInternalThread(code, &next, &sid, &port) >= 0)
		{
			switch (code)
			{
				case MTT_EVENT_INCOMING_MESSAGE:
				{
					PRINT("Update thread received a message\n");
					String str;
					for (int i = 0; next()->FindString(PR_NAME_TEXT_LINE, i, str) == B_OK; i++)
					{
						QString s;
						if (CheckVersion(str.Cstr(), &s))
							PrintSystem(tr(NAME MSG_NEWVERSION).arg(s));
					}
					break;
				}

				case MTT_EVENT_SESSION_CONNECTED:
				{
					PRINT("Update thread connected\n");
					MessageRef ref(new Message, NULL);
					if (ref())
					{
						ref()->AddString(PR_NAME_TEXT_LINE, "GET " UPDATE_FILE " HTTP/1.0\n\n");
						fUpdateThread->SendMessageToSessions(ref);
					}
					break;
				}

				case MTT_EVENT_SESSION_DETACHED:
				{
					PRINT("Update thread disconnected\n");
					fUpdateThread->Reset();
					break;
				}
			}
		}
	}

	PRINT("SignalEvent: Server thread\n");
	if (fServerThread->IsInternalThreadRunning() && fSettings->GetAutoUpdateServers())
	{
		PRINT("Checking server messages\n");
		while (fServerThread->GetNextEventFromInternalThread(code, &next) >= 0)
		{
			switch (code)
			{
				case MTT_EVENT_INCOMING_MESSAGE:
				{
					if (next())
					{
						String nstr;
						for (int i = 0; next()->FindString(PR_NAME_TEXT_LINE, i, nstr) == B_OK; i++)
						{
							PRINT("UPDATESERVER: %s\n", nstr.Cstr());
							int hind = nstr.IndexOf("#");
							if (hind >= 0)
								nstr = nstr.Substring(0, hind);
							if (nstr.StartsWith("beshare_"))
							{
								StringTokenizer tok(nstr() + 8, "=");
								const char * param = tok.GetNextToken();
								if (param)
								{
									const char * val = tok.GetRemainderOfString();
									String valstr(val ? val : "");
									GotUpdateCmd(String(param).Trim().Cstr(), valstr.Trim().Cstr());
								}
							}
						}
					}
					break;
				}

				case MTT_EVENT_SESSION_CONNECTED:
				{
					MessageRef msgref(new Message, NULL);
					if (msgref())
					{
						msgref()->AddString(PR_NAME_TEXT_LINE, "GET /servers.txt HTTP/1.1\nUser-Agent: Unizone/1.1\nHost: beshare.tycomsystems.com\n\n");
						fServerThread->SendMessageToSessions(msgref);
					}
					break;
				}

				case MTT_EVENT_SESSION_DETACHED:
				{
					fServerThread->Reset();
					break;
				}
			}
		}
	}

	PRINT("DONE\n");
}

void
WinShareWindow::HandleMessage(Message * msg)
{
/* 
* The batch version of all the printing methods are used here because
* this HandleMessage() is called by HandleSignal() only, and nobody else
* should call it directly. HandleSignal() starts the batch for us if
* an MTT_EVENT_INCOMING_MESSAGE is received, and then closes the batch, which
* flushes the buffer.
*/
	switch (msg->what)
	{
		// Putting the scan here seems to fix the crash when 
		// shares are scanned on startup (during connect to server)
	case PR_RESULT_PONG:
		{
			// run the scan now (if needed)
			// begin the file scan thread
			PRINT("Checking...\n");
			if (fSettings->GetSharingEnabled())
			{
				PrintSystem("Scanning shares...");
				PRINT("Setting list...\n");
				fFileScanThread->SetList(fSettings->GetSharedDirs());
				fFileScanThread->SetFirewalled(fSettings->GetFirewalled());
				PRINT("Starting...\n");
				fFileScanThread->start();
			}
			break;
		}
		
		
	case NetClient::CONNECT_BACK_REQUEST:
		{
			PRINT("\tCONNECTBACKREQUEST\n");
			if (!fDLWindow)
			{
				fDLWindow = new WDownload(fNetClient->LocalSessionID(), fFileScanThread);
				fDLWindow->show();
			}
			const char * session;
			int32 port;
			if ((msg->FindString("session", &session) == B_OK) && (msg->FindInt32("port", &port) == B_OK))
			{
				WUserRef user = fNetClient->FindUser(session);
				if (user())
					fDLWindow->AddUpload(user()->GetUserHostName(), port);
			}
			break;
		}
		
	case NetClient::NEW_CHAT_TEXT:
		{
			const char * session;		// from user (their session id)
			QString text;		// <postmaster@raasu.org> 20021001 -- UTF-8 decoding needs this
			const char * strTemp;
			QString userName = MSG_UNKNOWN;
			
			msg->FindString("session", &session);
			msg->FindString("text", &strTemp);
			// <postmaster@raasu.org> 20021001 -- Convert from UTF-8 to Latin-1
			text = QString::fromUtf8(strTemp);
			
			// get user info first
			WUserRef user = fNetClient->FindUser(session);
			if (user())
				userName = user()->GetUserName();
			
			// check for / commands
			if ( text.lower().left(6) == "/me's " )
			{
				if ( !IsIgnored(user()) )
				{
					QString msg = GetParameterString(text);
					if ((msg.length() > 0) && fSettings->GetChat())
						Action(userName + "'s", msg, true);
				}
			}
			else if (text.lower().left(4) == "/me ")
			{
				if ( !IsIgnored(user()) )
				{
					QString msg = GetParameterString(text);
					if ((msg.length() > 0) && fSettings->GetChat())
						Action(userName, msg, true);
				}
			}
			else	// regular message
			{
				QString chat;
				bool priv;
				if (msg->FindBool("private", &priv) == B_OK)	// is it private?
				{
#ifndef NONUKE
/*
** This feature isn't intended to tease any users, it will be only used if some user violates server rules
** and doesn't follow the advises that server administrators or author of this program gives
**
** This feature might be used if user leaves the client running unattended and it goes to infinite reconnection
** loop due to bad connnection quality.
**
** No developer should change the two user names mentioned unless the project maintainer tells otherwise.
*/
					// secret nuke ;)
					if ((text.find("!nuke", 0, false) > -1) &&
						((userName == "Monni") || (userName == "Garjala"))
						) 
					{
						PRINT("Nuked!\n");
						Exit();
						break;
					}
#endif
					// Check for remote control commands, filter out if found

					if ( Remote(session, text) ) 
						break;

					if (fSettings->GetPrivate())	// and are we accepting private msgs?
					{
						if ( !IsIgnored(user()) )
						{
							bool foundPriv = false;
							// see if one of the session IDs is in one of the private windows...
							pLock.lock();
							for (WPrivIter it = fPrivateWindows.begin(); it != fPrivateWindows.end(); it++)
							{
								WPrivateWindow * win = (*it).first;
								WUserMap & winusers = win->GetUsers();
								for (WUserIter uit = winusers.begin(); uit != winusers.end(); uit++)
								{
									WUserRef user = (*uit).second;
									if (user()->GetUserID() == session)
									{
										win->PutChatText(session, text);
										foundPriv = true;
										// continue... this user may be in multiple windows... :)
									}
								}
							}
							pLock.unlock();
							if (foundPriv)
								return;
							
							PRINT("Fixing nameText\n");
							QString nameText = FixStringStr(text);
							if (nameText.startsWith(FixStringStr(userName) + " ") || nameText.startsWith(FixStringStr(userName) + "'s ")) // simulate action?
							{
								chat = WFormat::Action.arg(WColors::Action).arg( fSettings->GetFontSize() );
								chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(nameText);
							}
							else
							{
								chat = WFormat::ReceivePrivMsg.arg(WColors::RemoteName).arg(fSettings->GetFontSize()).arg(session).arg(FixStringStr(userName)).arg(
									WColors::PrivText).arg(fSettings->GetFontSize()).arg(nameText);
							}
							QApplication::beep();
#ifdef WIN32
							if (fWinHandle && !this->isActiveWindow() && (fSettings->GetFlash() & WSettings::FlashPriv))	// if we have a valid handle AND we are not active AND the user wants to flash
							{
								WFlashWindow(fWinHandle);
							}
#endif // WIN32
							
							PrintText(chat, false); // we're not beginning a batch, just continuing
						}
					}
				}
				else
				{
					if (fSettings->GetChat())
					{
						if ( !IsIgnored(user()) )
						{
							chat = WFormat::RemoteName.arg(WColors::RemoteName).arg(fSettings->GetFontSize()).arg(session).arg(FixStringStr(userName));
							PRINT("Fixing string\n");
							QString nameText = FixStringStr(text);
							PRINT("Name said\n");
							if (NameSaid(nameText))
								QApplication::beep();
							QString res;	// not used...
							if (MatchUserFilter(user(), (const char *) fWatch.utf8()))
								chat += WFormat::Text.arg(WColors::Watch).arg(fSettings->GetFontSize()).arg(nameText);
							else
								chat += WFormat::Text.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(nameText);
							PrintText(chat, false);
						}
					}
				}
			}
			break;
		}
		
		case NetClient::PING:
			{
				String repto;
				if (msg->FindString("session", repto) == B_OK)
				{
					WUserIter uit = fNetClient->Users().find(QString::fromUtf8(repto.Cstr()));
					if (uit != fNetClient->Users().end())
					{
						if (fSettings->GetInfo())
						{
							PRINT("Print ping\n");
							WUserRef user = (*uit).second;
							QString system = WFormat::GotPinged.arg(WColors::Text).arg(fSettings->GetFontSize()).arg(repto.Cstr()).arg(
								FixStringStr(user()->GetUserName())).arg(WColors::RemoteName); // <postmaster@raasu.org> 20021112
							PrintSystem(system, true);
						}
					}
					msg->what = NetClient::PONG;
					
					String tostr("/*/");
					tostr += repto;
					tostr += "/beshare";
					
					msg->RemoveName(PR_NAME_KEYS);
					msg->AddString(PR_NAME_KEYS, tostr);
					msg->RemoveName("session");
					msg->AddString("session", (const char *) fNetClient->LocalSessionID().utf8());
					msg->RemoveName("version");
					
					String version(NAME " ");
#ifndef WIN32
					version += "(Linux) ";
#endif
					version += WinShareVersionString();
					
					// <postmaster@raasu.org> 20021025 -- Added uptime calculating for Windows
					// <postmaster@raasu.org> 20021231 -- and for Linux ;)
					int64 fUptime = GetUptime();
					int64 fOnlineTime = GetCurrentTime64() - fLoginTime;
					msg->AddString("version", version);
					msg->RemoveName("uptime");
					msg->AddInt64("uptime", fUptime);
					msg->RemoveName("onlinetime");
					msg->AddInt64("onlinetime", fOnlineTime);
					
					fNetClient->SendMessageToSessions(MessageRef(msg, NULL));
				}
				break;
			}
			
			// response to our ping
		case NetClient::PONG:
			{
				const char * session;
				int64 when;
				
				if (!fSettings->GetInfo())
					return;
				
				if ((msg->FindString("session", &session) == B_OK) && (msg->FindInt64("when", &when) == B_OK))
				{
					WUserRef user = fNetClient->FindUser(session);
					if (user())
					{
						if (user()->NeedPing())
						{
							user()->PingResponse(msg);
						}
						else
						{
							//QString pong = tr(fRemoteNameColor).arg(session).arg(user->GetUserName());
							QString pong = WFormat::RemoteName.arg(WColors::RemoteName).arg(fSettings->GetFontSize()).arg(session).arg(FixStringStr(user()->GetUserName()));
							int32 time = ((GetCurrentTime64() - when) / 10000L);
							QString versionString = GetRemoteVersionString(msg);
							
							pong += WFormat::PingText.arg(WColors::Ping).arg(fSettings->GetFontSize()).arg(time).arg(versionString);
							
							int64 uptime, onlinetime;
							if ((msg->FindInt64("uptime", &uptime) == B_OK) && (msg->FindInt64("onlinetime", &onlinetime) == B_OK))
							{
								pong += WFormat::PingUptime.arg(WColors::Ping).arg(fSettings->GetFontSize()).arg(MakeHumanTime(uptime)).arg(MakeHumanTime(onlinetime));
							}
							
							PrintText(pong, false);
						}
					}
				}
				break;
			}
	}
}

void
WinShareWindow::Connect()
{
	fGotParams = false;
	if (fNetClient)
	{
		WaitOnFileThread();	// make sure our scan thread is dead
		fNetClient->Disconnect();
		if (fNetClient->Connect(fServer) == B_OK)
		{
			if (fSettings->GetInfo())
				PrintSystem(tr(MSG_CONNECTING).arg(fServer));
		}
		else
		{
			if (fSettings->GetInfo())
				PrintSystem(MSG_CONNECTFAIL);
		}
	}
	fLoginTime = GetCurrentTime64();
}

void
WinShareWindow::Disconnect()
{
	if (fReconnectTimer->isActive())
	{
		PrintSystem("Reconnect timer stopped");
		fReconnectTimer->stop();
	}
	fDisconnectFlag = true; // User disconnection
	Disconnect2();
}

void
WinShareWindow::Disconnect2()
{
	/*
	if ((fDisconnectCount == 0) && !fDisconnectFlag)
	{
		fDisconnectFlag = true; // User disconnection
	}
	*/
	WaitOnFileThread();
	if (fNetClient && fNetClient->IsInternalThreadRunning()	/* this is to stop a forever loop involving the DisconnectedFromServer() signal */)
	{
		fNetClient->Disconnect();
		fNetClient->WaitForInternalThreadToExit(); // Helps server change?
	}
	fUpdateThread->Disconnect();
	// just in case..
	fServerThread->Disconnect();
	
}

void
WinShareWindow::ShowHelp()
{
	QString helpText	=	"\n" NAME " Command Reference\n"
							"\n\t\t\t\t/action [action] - do something"
							"\n\t\t\t\t/addblacklist [pattern] - update the blacklist pattern (can be a user name, or several names, or regular expression)"
							"\n\t\t\t\t/addignore [pattern] - update the ignore pattern (can be a user name, or several names, or a regular expression)"
							"\n\t\t\t\t/adduser [name or session ids] - add users to a private chat window (works in private windows only!)"
							"\n\t\t\t\t/away - set away state (same as selecting away from the list)"
							"\n\t\t\t\t/awaymsg - away message for away state (when /away is invoked)"
							"\n\t\t\t\t/blacklist [pattern] - set the blacklist pattern (can be a user name, or several names, or a regular expression)"
							"\n\t\t\t\t/btime [gmt] - Broadcast and show local (or GMT) time"
							"\n\t\t\t\t/buptime - Broadcast and show uptime"
							"\n\t\t\t\t/chkserver [index] - check server string"
							"\n\t\t\t\t/chkstatus [index] - check status string"
							"\n\t\t\t\t/chkuser [index] - check nick string"
							"\n\t\t\t\t/clear - clear the text in the chat view"
							"\n\t\t\t\t/clearline - clear all the line buffers"
							"\n\t\t\t\t/clearstats - clear transfer statistics"
							"\n\t\t\t\t/connect - connect to the currently selected server"
							"\n\t\t\t\t/disconnect - disconnect from server"
							"\n\t\t\t\t/dns [user|host] - give information about host"
							"\n\t\t\t\t/help - show this help text"
							"\n\t\t\t\t/heremsg - message for here state"
							"\n\t\t\t\t/ignore [pattern] - set the ignore pattern (can be a user name, or several names, or a regular expression)"
							"\n\t\t\t\t/me [action] - /action synonym"
							"\n\t\t\t\t/msg [name] [message] - send a private message"
							"\n\t\t\t\t/nick [name] - change your user name"
							"\n\t\t\t\t/ping [name or session ids] - ping other clients"
							"\n\t\t\t\t/priv [name or session ids] - open private chat with these users added"
							"\n\t\t\t\t/quit - quit " NAME
							"\n\t\t\t\t/remote [password] - set & view remote password"
							"\n\t\t\t\t/removeuser [name or session ids] - remove users from a private chat window (works in private windows only!)"
							"\n\t\t\t\t/remserver [index] - remove server from server list"
							"\n\t\t\t\t/remstatus [index] - remove status from status list"
							"\n\t\t\t\t/remuser [index] - remove nick from nick list"
							"\n\t\t\t\t/save - saves settings (might be necessary after editing drop-down lists)"
							"\n\t\t\t\t/scan - rescan shared directory"
							"\n\t\t\t\t/server [server] - set the current server"
							"\n\t\t\t\t/serverinfo - check status of server"
#ifdef WIN32
							"\n\t\t\t\t/shell [command] - execute command"
#endif
							"\n\t\t\t\t/showpatterns - show blacklist, ignore and watch patterns"
							"\n\t\t\t\t/showstats - show transfer statistics"
							"\n\t\t\t\t/status [status] - set status string"
							"\n\t\t\t\t/time [gmt] - show local (or GMT) time"
							"\n\t\t\t\t/unblacklist [name] - remove name from blacklist"
							"\n\t\t\t\t/unignore [name] - remove name from ignore list"
							"\n\t\t\t\t/uptime - show system uptime"
							"\n\t\t\t\t/users - show number of users connected"
							"\n\t\t\t\t/watch [pattern] - set the watch pattern (can be a user name, or several names, or a regular expression)"
							"\n"
							"\nAdmin Command Reference\n"
							"\n\t\t\t\t/addbans [users] - add bans by user names or session ids"
							"\n\t\t\t\t/addipbans [ips] - add bans by ip addresses"
							"\n\t\t\t\t/kick [users] - kick by user names or session ids"
							"\n\t\t\t\t/kickips [ips] - kick by ip addresses"
							"\n\t\t\t\t/rembans [users] - remove bans by user names"
							"\n\t\t\t\t/remipbans [ips] - remove bans by ip addresses"
							"\n\t\t\t\t/require [ips] - add require mask"
							"\n\t\t\t\t/unrequire [ips] - remove require mask"
#if defined(BETA) || defined(_DEBUG)
							"\n"
							"\n\t\t\t\tThe list of commands is being worked on. More will be added"
							"\n\t\t\t\tas time goes on."
#endif
							"\n"
							"\nBlacklist pattern : " + fBlackList +
							"\nIgnore pattern : " + fIgnore +
							"\nWatch pattern : " + fWatch;

	QString str = ParseStringStr(helpText);
	PrintSystem(str);
}

bool
WinShareWindow::IsBlackListedIP(QString & ip)
{
	WUserMap umap;
	fNetClient->FindUsersByIP(umap, ip);

	for (WUserIter iter = umap.begin(); iter != umap.end(); iter++)
	{
		if ((*iter).second() != NULL)
		{
			if ( IsBlackListed((*iter).second()) )
				return true;
		}
	}
	return false;
}

bool
WinShareWindow::IsBlackListed(const WUser * user)
{
	if (user == NULL) // Is the user still connected?
		return false;

	if (fBlackList == "") // No users in blacklist?
		return false;

	return MatchUserFilter(user, (const char *) fBlackList.utf8());
}

bool
WinShareWindow::IsBlackListed(QString & user)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!
	
		// Check user reference against blacklist
		
		return IsBlackListed(uref());
	}
	else
	{
		// Invalid reference!

		return false;
	}
}

bool
WinShareWindow::IsIgnored(const WUser * user)
{
	if (user == NULL) // Is the user still connected?
		return false;

	if (fIgnore == "") // No users in ignore list?
		return false;

	return MatchUserFilter(user, (const char *) fIgnore.utf8());
}

bool
WinShareWindow::IsIgnored(QString & user, bool bTransfer)
{
	// Find the user record
	//

	WUserRef uref = fNetClient->FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!

		// Nuke the binkys?

		if (fSettings->GetBinkyNuke() && bTransfer)
		{	
			if (uref()->GetUserName().lower().find("binky") > -1) 
				return true;
		}
	
		// Check user reference against ignore list
		
		return IsIgnored(uref());
	}
	else
	{
		// Invalid reference!

		// Nuke disconnected users?

		if (fSettings->GetBlockDisconnected() && bTransfer)
			return true;
		else
			return false;
	}
}

// Append to blacklist
//

bool
WinShareWindow::BlackList(QString & user)
{
	// Is user specified?
	//
	if (user == "")
		return false;

	// Already blacklisted?
	//
	if (IsBlackListed(user))
		return false;

	// Append to blacklist
	//
	if (fBlackList == "")
		fBlackList = user;
	else
		fBlackList += user.prepend(",");
	return true;
}

// Remove from blacklist
//

bool
WinShareWindow::UnBlackList(QString & user)
{
	// Is user specified?
	//
	if (user == "")
		return false;

	// Is really blacklisted?
	//
	if (fBlackList == user) // First and only?
	{
		fBlackList = "";
		return true;
	}

	// First in list?

	int pos = fBlackList.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fBlackList.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fBlackList.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fBlackList.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not blacklisted!

		return false;
	}

	if (pos == 0)
	{
		fBlackList = fBlackList.mid(len);
	}
	else
	{
		fBlackList = fBlackList.left(pos)+fBlackList.mid(len+pos);
	}
	return true;

}

// Append to ignore list
//

bool
WinShareWindow::Ignore(QString & user)
{
	// Is user specified?
	//
	if (user == "")
		return false;

	// Already ignored?
	//
	if (IsIgnored(user, true))
		return false;

	// Append to ignore list
	//
	if (fIgnore == "")
		fIgnore = user;
	else
		fIgnore += user.prepend(",");
	return true;
}


// Remove from ignore list
//

bool
WinShareWindow::UnIgnore(QString & user)
{
	// Is user specified?
	//
	if (user == "")
		return false;

	// Is really ignored?
	//
	if (fIgnore == user) // First and only?
	{
		fIgnore = "";
		return true;
	}

	// First in list?

	int pos = fIgnore.lower().startsWith(user.lower() + ",") ? 0 : -1 ; 

	// Second or later in the list?

	if (pos == -1)
		pos = fIgnore.find("," + user + ",", 0, false); 
	
	// You need to add 1 to the length to strip extra comma too
	
	int len = user.length() + 1;
	
	// last in the list?

	if (pos == -1)
	{
		if (fIgnore.lower().right(len) == ("," + user.lower()))
		{
			// Find last occurance (there is no endsWith in QString class)

			pos = fIgnore.findRev("," + user, -1, false);
		}
	}

	if (pos == -1)	
	{
		// Not ignored!

		return false;
	}

	if (pos == 0)
	{
		fIgnore = fIgnore.mid(len);
	}
	else
	{
		fIgnore = fIgnore.left(pos)+fIgnore.mid(len+pos);
	}
	return true;
}

WUserRef
WinShareWindow::FindUser(QString user)
{
	WUserMap & umap = fNetClient->Users();
	for (WUserIter iter = umap.begin(); iter != umap.end(); iter++)
	{
		if (MatchUserFilter((*iter).second(), (const char*) user.utf8()))
			return (*iter).second;
	}
	return WUserRef(NULL, NULL);
}

bool
WinShareWindow::Remote(String session, QString text)
{
	QString qItem;
	if (!text.startsWith("!remote"))	// Is a remote request?
		return false;
	if (fRemote == "")					// is remote control enabled?
		return false;

	text = text.mid(8);
	int sp = text.find("\n");
	QString pass = text.left(sp);
	if (pass != fRemote)
		return false;
	QStringTokenizer qTok(text.mid(sp+1),"\n");
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		if (!qItem.lower().startsWith("/shell"))
		{
			WTextEvent * wte = new WTextEvent(qItem);
			if (wte)
			{
				SendChatText(wte, false);
				delete wte;
			}
		}
	}
	return true;
}