// An extension of the main WinShare class
// Contains all the networking related methods

#ifdef WIN32
#pragma warning(disable: 4786)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
typedef hostent *LPHOSTENT;
#endif

#include "winsharewindow.h"
#include "global.h"
#include "version.h"
#include "settings.h"
#include "debugimpl.h"
#include "formatting.h"
#include "textevent.h"
#include "platform.h"
#include "picviewerimpl.h"
#include "util.h"
#include "wstring.h"
#include "gotourl.h"			// <postmaster@raasu.org> 20021116 -- for /shell
#include "tokenizer.h"			// <postmaster@raasu.org> 20030902 -- for !remote
#include "ulistview.h"
#include "downloadimpl.h"
#include "filethread.h"
#include "netclient.h"
#include "serverclient.h"
#include "updateclient.h"
#include "wstatusbar.h"
#include "textevent.h"

#include "util/TimeUtilityFunctions.h"
#include "util/StringTokenizer.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "iogateway/MessageIOGateway.h"
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
#include "system/GlobalMemoryAllocator.h"
#endif

#include <time.h>				//                                 -- for /time
#include <qapplication.h>
#include <qfiledialog.h>

#if (QT_VERSION >= 0x030000)
#include <qregexp.h>
#endif

void TextEvent(QObject *target, const QString &text, WTextEvent::Type t);
void AddToList(QString &slist, const QString &entry);

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
				if (fNetClient->IsConnected())	// are we connected?
				{
					if (!fFileScanThread->IsInternalThreadRunning())
					{
						StartAcceptThread();
						ScanShares(true);
					}
					else
					{
						if (fSettings->GetError())
							PrintError(tr("Already scanning!"));
					}
				}
				else
				{
					if (fSettings->GetError())
						PrintError(tr("Not connected."));
				}
			}
			else
			{
				if (fSettings->GetError())
					PrintError(tr("File sharing not enabled."));
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
				if (name.find(QString("binky"), 0, false) >= 0)
				{
					if (fSettings->GetError())
						PrintError(tr("Invalid nickname!"));
					return;
				}
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
					PrintError(tr("No nickname passed."));
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
					PrintError(tr("No message to send."));
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
					PrintError(tr("No message to send."));
			}
		}
		else if (sendText.left(2) == "//")	// used so that / commands can be printed
		{
			sendText.replace(0, 2, "/");
			SendChatText("*", sendText);
		}
		else if (CompareCommand(sendText, "/help"))
		{
			QString command = GetParameterString(sendText);
			if (fSettings->GetInfo())
				ShowHelp(command);
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
//				bool found = false;

				int i;	// this way we know which one was found

				for (i = 0; i < fStatusList->count(); i++)
				{
					QString status = fStatusList->text(i).stripWhiteSpace();
					if (status.lower() == ns.lower())
					{
						fStatusList->setCurrentItem(i);
						StatusChanged(fStatusList->currentText());
						return;
					}
				}

				fStatusList->insertItem(ns, 0); // <postmaster@raasu.org> 20021005
				fStatusList->setCurrentItem(0);
				StatusChanged(ns);
			}
		}
		else if (CompareCommand(sendText, "/serverinfo"))
		{
			fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_GETPARAMETERS));
		}
		else if (CompareCommand(sendText, "/server"))
		{
			QString server = GetParameterString(sendText);

			if (server.length() > 0)
			{
				QString serverStr = server.stripWhiteSpace();

				// the easy way to add the server to the list...
				// this will check for duplicates
				GotUpdateCmd("addserver", serverStr);	
				// now find our server
				for (int i = 0; i < fServerList->count(); i++)
				{
					QString slist = fServerList->text(i).stripWhiteSpace();
					if (slist.lower() == serverStr.lower())
					{
						// found our server
						fServerList->setCurrentItem(i);
						ServerChanged(fServerList->currentText());
						return;
					}
				}
			}
		}
		else if (CompareCommand(sendText, "/connect"))
		{
			Connect();
		}
		else if (CompareCommand(sendText, "/compression"))
		{
			QString enc = GetParameterString(sendText);
			uint32 _en = fSettings->GetEncoding(GetServerName(fServer), GetServerPort(fServer)); // Current value
			if (enc.length() > 0)
			{
				bool b;
				_en = enc.toULong(&b);
				if (b)
				{
					fStatusBar->setText(tr( "Current compression: %1" ).arg(_en), 1);

					int _en2 = _en + MUSCLE_MESSAGE_ENCODING_DEFAULT;
					if (_en2 >= MUSCLE_MESSAGE_ENCODING_END_MARKER)
					{
						if (fSettings->GetError())
							PrintError(tr("Invalid compression!"));
					}
					else
					{
						fSettings->SetEncoding(GetServerName(fServer), GetServerPort(fServer), _en2);
						if (fNetClient->IsConnected())
						{
							fNetClient->SetOutgoingMessageEncoding(_en2);
						}
						if (fSettings->GetInfo())
						{
							// Make the hyperlink contain port number too ;)
							QString uri = "server://"; 
							uri += fServer;
							uri += " [";
							uri += GetServerName(fServer);
							uri += "]";
							//
							PrintSystem( tr("Compression level for server %1 at port %2 set to %3.").arg(uri).arg(GetServerPort(fServer)).arg(_en) );
						}
					}
				}
				else
				{
					if (fSettings->GetError())
						PrintError(tr("Invalid compression!"));
				}
			}
			else
			{
				if (fSettings->GetInfo())
					PrintSystem(tr("Current compression: %1").arg(_en - MUSCLE_MESSAGE_ENCODING_DEFAULT));
			}
		}
		else if (CompareCommand(sendText, "/disconnect"))
		{
			Disconnect();
		}
		else if (CompareCommand(sendText, "/awaymsg"))
		{
			QString msg = GetParameterString(sendText);

			if (msg.length() > 0)
			{
				fAwayMsg = msg;
				if (fSettings->GetInfo())
					PrintSystem(tr("Away message set to %1.").arg(fAwayMsg));
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

			if (msg.length() > 0)
			{
				fHereMsg = msg;
				if (fSettings->GetInfo())
					PrintSystem(tr("Here message set to %1.").arg(fHereMsg));
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
		else if (CompareCommand(sendText, "/running"))
		{
			PrintSystem( tr("Running: %1").arg(MakeHumanTime(GetCurrentTime64() - GetStartTime())) );
		}
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
				SendChatText("*", text);
		}
		else if (CompareCommand(sendText, "/logged"))
		{
			if (fNetClient->IsConnected())	// are we connected?
			{
				PrintSystem( tr("Logged In: %1").arg(MakeHumanTime(GetCurrentTime64() - fLoginTime)) );
			}
			else
			{
				if (fSettings->GetError())
					PrintError(tr("Not connected."));
			}		
		}
		// <postmaster@raasu.org> 20020929
		else if (CompareCommand(sendText, "/users"))
		{
			int iUsers = fUsers->childCount() + 1; // <postmaster@raasu.org> 20021005 -- I think it's 1 for current user that needs to be added to get the total count
			PrintSystem(tr("Number of users logged in: %1").arg(iUsers));
		}
		else if (CompareCommand(sendText, "/priv"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
					LaunchPrivate(users);
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/kick"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_KICK));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/kickips"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_KICK));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapIPsToNodes(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/addbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDBANS));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/addipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDBANS));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapIPsToNodes(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/rembans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEBANS));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapUsersToIDs(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/remipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEBANS));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapIPsToNodes(users).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
		}
		else if (CompareCommand(sendText, "/require"))
		{
			QString mask = GetParameterString(sendText);

			if (mask.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDREQUIRES));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapIPsToNodes(mask).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/unrequire"))
		{
			QString mask = GetParameterString(sendText);

			if (mask.length() > 0)
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEREQUIRES));
				mrf()->AddString(PR_NAME_KEYS, (const char *) MapIPsToNodes(mask).utf8());
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		// <postmaster@raasu.org> 20021116
		else if (CompareCommand(sendText, "/shell"))
		{
			QString command = GetParameterString(sendText);

			if (command.length() > 0)
			{
				RunCommand(command);
			}
		}
		else if (CompareCommand(sendText, "/time"))
		{
			QString command = GetParameterString(sendText).lower();
			String lt;
			time_t currentTime = time(NULL);
			tm * myTime;
			char zone[64];
			QString tuser = QString::null;

			if ((command.length() > 0) && (command != "gmt")) // User name?
			{
				int rpos;
				if (command.left(1) == "'")
				{
					rpos = command.find("'",1);
					if (rpos >= 0)
					{
						tuser = command.mid(1,rpos-1);
						if ((int) command.length() > (rpos + 3))
							command = command.mid(rpos + 2);
						else
							command = QString::null;
					}
				}
				else
				{
					rpos = command.find(" ");
					if (rpos == -1)
					{
						rpos = command.length();
						tuser = command;
						command = QString::null;
					}
					else
					{
						tuser = command.left(rpos);
						command = command.mid(rpos + 1);
					}
				}
			}

			if (tuser == QString::null)
			{
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
				
				PrintSystem(tr("Current time: %1 %2").arg(QString::fromLocal8Bit(lt.Cstr())).arg(QString::fromLocal8Bit(zone)));
			}
			else
			{
				WUserRef tu = FindUser(tuser);
				if (tu())
				{
					String to("/*/");
					to += (const char *) tu()->GetUserID().utf8();
					to += "/unishare";

					MessageRef tire(GetMessageFromPool(TimeRequest));
					if (tire())
					{
						tire()->AddString(PR_NAME_KEYS, to);
						tire()->AddString(PR_NAME_SESSION, (const char *) GetUserID().utf8());
						if (command == "gmt")
						{
							tire()->AddBool("gmt", true);
						}
						if (fSettings->GetInfo())
						{
							// QString userid = tu()->GetUserID();
							QString username = tu()->GetUserName();
							QString treqMsg = WFormat::TimeRequest(tu()->GetUserID(), FixStringStr(username));
							PrintSystem(treqMsg);
						}

						fNetClient->SendMessageToSessions(tire);
					}		
				}
				else // Invalid user?
				{
					if (fSettings->GetError())
						PrintError(tr("User(s) not found!"));

				}
			}
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

			QString text = tr("Current time: %1 %2").arg(QString::fromLocal8Bit(lt.Cstr())).arg(QString::fromLocal8Bit(zone));

			if (fNetClient->IsConnected())
				SendChatText("*", text);
		}
		else if (CompareCommand(sendText, "/ignore"))
		{
			fIgnore = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fIgnore.isEmpty())
					PrintSystem(tr("Ignore pattern cleared."));
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
				PrintSystem(tr("Ignore pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating ignore pattern!" ) );
		}
		else if (CompareCommand(sendText, "/unignore"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnIgnore(user);

			if (bSuccess)
				PrintSystem(tr("Ignore pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating ignore pattern!" ) );
		}
		else if (CompareCommand(sendText, "/blacklist"))
		{
			fBlackList = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fBlackList.isEmpty())
					PrintSystem(tr("Blacklist pattern cleared."));
				else
					PrintSystem(tr("Blacklist pattern set to: %1").arg(fBlackList));
			}
		}
		else if (CompareCommand(sendText, "/whitelist"))
		{
			fWhiteList = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fWhiteList.isEmpty())
					PrintSystem(tr("Whitelist pattern cleared."));
				else
					PrintSystem(tr("Whitelist pattern set to: %1").arg(fBlackList));
			}
		}
		else if (CompareCommand(sendText, "/filter"))
		{
			fFilterList = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fFilterList.isEmpty())
					PrintSystem(tr("Filter list pattern cleared."));
				else
					PrintSystem(tr("Filter list pattern set to: %1").arg(fFilterList));
			}
		}
		else if (CompareCommand(sendText, "/addblacklist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = BlackList(user);

			if (bSuccess)
				PrintSystem(tr("Blacklist pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating blacklist pattern!" ) );
		}
		else if (CompareCommand(sendText, "/addwhitelist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = WhiteList(user);

			if (bSuccess)
				PrintSystem(tr("Whitelist pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating whitelist pattern!" ) );
		}
		else if (CompareCommand(sendText, "/addfilter"))
		{
			bool bSuccess;
			QString pattern = GetParameterString(sendText);

			bSuccess = FilterList(pattern);

			if (bSuccess)
				PrintSystem(tr("Filter list pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating filter list pattern!" ) );
		}		
		else if (CompareCommand(sendText, "/unblacklist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnBlackList(user);

			if (bSuccess)
				PrintSystem(tr("Blacklist pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating blacklist pattern!" ) );
		}
		else if (CompareCommand(sendText, "/unwhitelist"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnWhiteList(user);

			if (bSuccess)
				PrintSystem(tr("Whitelist pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating whitelist pattern!" ) );
		}
		else if (CompareCommand(sendText, "/unfilter"))
		{
			bool bSuccess;
			QString pattern = GetParameterString(sendText);

			bSuccess = UnFilterList(pattern);

			if (bSuccess)
				PrintSystem(tr("Filter list pattern updated."));
			else if (fSettings->GetError())
				PrintError( tr( "Error updating filter list pattern!" ) );
		}
		else if (CompareCommand(sendText, "/autopriv"))
		{
			fAutoPriv = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fAutoPriv.isEmpty())
					PrintSystem(tr("Auto-private pattern cleared."));
				else
					PrintSystem(tr("Auto-private pattern set to: %1").arg(fAutoPriv));
			}
		}
		else if (CompareCommand(sendText, "/addautopriv"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = AutoPrivate(user);

			if (bSuccess)
				PrintSystem( tr( "Auto-private pattern updated." ) );
			else if (fSettings->GetError())
				PrintError( tr( "Error updating auto-private pattern!" ) );
		}
		else if (CompareCommand(sendText, "/unautopriv"))
		{
			bool bSuccess;
			QString user = GetParameterString(sendText);

			bSuccess = UnAutoPrivate(user);

			if (bSuccess)
				PrintSystem( tr( "Auto-private pattern updated." ) );
			else if (fSettings->GetError())
				PrintError( tr( "Error updating auto-private pattern!" ) );
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No index specified."));
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No users passed."));
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No index specified."));
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
					PrintError(tr("Invalid index."));
			}
			else if (fSettings->GetError())
				PrintError(tr("No index specified."));
		}
		else if (CompareCommand(sendText, "/save"))
		{
			PrintSystem(tr("Saving configuration..."));
			SaveSettings();
			PrintSystem(tr("Configuration saved."));
		}		
		else if (CompareCommand(sendText, "/dns"))
		{
			QString user = GetParameterString(sendText);
			GetAddressInfo(user, true);
		}
		else if (CompareCommand(sendText, "/ip"))
		{
			QString user = GetParameterString(sendText);
			GetAddressInfo(user, false);
		}
		else if (CompareCommand(sendText, "/showstats"))
		{
			PrintSystem( tr("Uploaded: This session %1, total %2").arg(MakeSizeString(tx-tx2)).arg(MakeSizeString(tx)) );
			PrintSystem( tr("Downloaded: This session %1, total %2").arg(MakeSizeString(rx-rx2)).arg(MakeSizeString(rx)) );
		}
		else if (CompareCommand(sendText, "/showpatterns"))
		{
			QString temp;
			QString qNone = tr("&lt;None&gt;");

			// Auto-private
			//
			temp = CheckIfEmpty(fAutoPriv, qNone);

			PrintSystem( tr("Auto-private pattern: %1").arg(temp) );
			
			// Black List
			//
			temp = CheckIfEmpty(fBlackList, qNone);

			PrintSystem( tr("Blacklist pattern: %1").arg(temp) );

			// Filter List
			temp = CheckIfEmpty(fFilterList, qNone);

			PrintSystem( tr("Filter list pattern: %1").arg(temp) );

			// Ignore
			//
			temp = CheckIfEmpty(fIgnore, qNone);

			PrintSystem( tr("Ignore pattern: %1").arg(temp) );

			// Watch
			//
			temp = CheckIfEmpty(fWatch, qNone);

			PrintSystem( tr("Watch pattern: %1").arg(temp) );

			// White List

			temp = CheckIfEmpty(fWhiteList, qNone);

			PrintSystem( tr("Whitelist pattern: %1").arg(temp) );

			// On Connect
			//
			if (fOnConnect.isEmpty())
			{
				PrintSystem( tr("On Connect: Do Nothing ;)" ) );
			}
			else
			{
				PrintSystem( tr("On Connect:") );
				PrintSystem( tr("1. %1").arg(fOnConnect) );
				if (!fOnConnect2.isEmpty())
					PrintSystem( tr("2. %1").arg(fOnConnect2) );
			}
		}
		else if (CompareCommand(sendText, "/clearstats"))
		{
			tx = rx = 0;
			tx2 = rx2 = 0;
			PrintSystem(tr("Transfer statistics cleared."));
		}
		else if (CompareCommand(sendText, "/remote"))
		{
			QString p = GetParameterString(sendText);
			if (p.isEmpty())
				PrintSystem( tr("Remote password: %1").arg(fRemote) );
			else
			{
				fRemote = p;
				PrintSystem( tr("Remote password set to: %1").arg(p) );
			}	
		}
		else if (CompareCommand(sendText, "/resumes"))
		{
			ListResumes();
		}
		else if (CompareCommand(sendText, "/clearresumes"))
		{
			ClearResumes();
		}
		else if (CompareCommand(sendText, "/version"))
		{
			PrintSystem(tr("Unizone version: %1").arg(WinShareVersionString()));
			PrintSystem(tr("MUSCLE version: %1").arg(MUSCLE_VERSION_STRING));
		}
		else if (CompareCommand(sendText, "/onconnect"))
		{
			fOnConnect = GetParameterString(sendText);
			if (fOnConnect.isEmpty())
			{
				PrintSystem( tr("On Connect: Do Nothing ;)" ) );
			}
			else
			{
				PrintSystem( tr("On Connect:") );
				PrintSystem( tr("1. %1").arg(fOnConnect) );
				if (!fOnConnect2.isEmpty())
					PrintSystem( tr("2. %1").arg(fOnConnect2) );
			}
		}
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
		else if (CompareCommand(sendText, "/memory"))
		{
			PrintSystem(tr("Memory used: %1 bytes").arg(GetNumAllocatedBytes()));
		}
#endif
		else if (CompareCommand(sendText, "/search"))
		{
			QString pattern = GetParameterString(sendText);
			LaunchSearch(pattern);
		}
		else if (CompareCommand(sendText, "/reverse"))
		{
			QString qtext = GetParameterString(sendText);
			Reverse(qtext);
			if (fNetClient->IsConnected())
				SendChatText("*", qtext);
		}
		else if (CompareCommand(sendText, "/revsay"))
		{
			QString qtext = GetParameterString(sendText);
			int cp = qtext.find(": ");
			if (cp >= 0)
			{
				QString who = qtext.left(cp + 2);
				QString rtext = qtext.mid(cp + 2);
				Reverse(rtext);
				qtext = who;
				qtext += rtext;
			}
			else
			{
				Reverse(qtext);
			}
			if (fNetClient->IsConnected())
				SendChatText("*", qtext);
		}
		else if (CompareCommand(sendText, "/view"))
		{
			QString file = QFileDialog::getOpenFileName ( "downloads/", "*.png;*.bmp;*.xbm;*.xpm;*.pnm;*.jpg;*.jpeg;*.mng;*.gif", this);
			if (!file.isEmpty())
			{
				if (fPicViewer->LoadImage(file))
				{
					fPicViewer->show();
				}
				else
				{
					GotoURL(file);
				}
			}
		}
		else if (CompareCommand(sendText, "/picture"))
		{
			QString users = GetParameterString(sendText);
			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
				return;
			}
			int numMatches;
			WUserMap wmap;
			numMatches = FillUserMap(users, wmap);
			if (numMatches > 0)	// Found atleast one match in users
			{
				QString list;
				WUserIter uiter = wmap.begin();
				while (uiter != wmap.end())
				{
					AddToList(list, (*uiter).first);
					uiter++;
				}
				
				QString file = QFileDialog::getOpenFileName ( "downloads/", "*.png;*.bmp;*.xbm;*.xpm;*.pnm;*.jpg;*.jpeg;*.mng;*.gif", this);
				
				if (!file.isEmpty())
				{
					QFile fFile(file);
					if (fFile.open(IO_ReadOnly))
					{
						ByteBufferRef buf = GetByteBufferFromPool();
						if (buf())
						{
							if (buf()->SetNumBytes(fFile.size(), false) == B_OK)
							{
								fFile.readBlock((char *) buf()->GetBuffer(), fFile.size());
								fFile.close();
								QFileInfo info(file);
								fNetClient->SendPicture(list, buf, info.fileName());
							}
						}
					}
				}
			}
			else
			{
				if (gWin->fSettings->GetError())
					PrintError( tr( "User(s) not found!" ) );
			}
		}

		//
		// add more commands BEFORE this one
		//

		else if (sendText.left(1) == "/")
		{
			// unknown command...
			if (reply)
			{
				*reply = true;
				// format an error string
				sendText = WFormat::Error( tr("Unknown command!") );
				e->SetText(sendText);
			}
			else
			{
				if (fSettings->GetError())
					PrintError(tr("Unknown command!"));
			}
		}
		else
		{
			SendChatText("*", sendText);
		}
	}
}

void
WinShareWindow::SendChatText(const QString & sid, const QString & txt)
{
	fNetClient->SendChatText(sid, txt);
	QString out = FixStringStr(txt);
	if (sid == "*")	// global message?
	{
		if (fSettings->GetChat())
		{
			QString chat = WFormat::LocalText(GetUserID(), FixStringStr(GetUserName()), out);
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
WinShareWindow::SendChatText(const QString & sid, const QString & txt, const WUserRef & priv, bool * reply)
{
	fNetClient->SendChatText(sid, txt);
	QString out = FixStringStr(txt);
	if (sid == "*")	// not a global message?
	{
		if (fSettings->GetChat())
		{
			QString chat = WFormat::LocalText(GetUserID(), FixStringStr(GetUserName()), out);
			PrintText(chat);
		}
	}
	else
	{
		if (priv())
		{
			// "reply" is handled here...
			if (!reply)	// no reply requested? print text out
			{
				if (fSettings->GetPrivate())	// do we take private messages?
				{
					QString chat;
					QString me = GetUserName();
					FixString(me);
					QString other = priv()->GetUserName();
					FixString(other);
					PRINT("Appending to chat\n");
					if ( IsAction(txt, me) ) // simulate action?
					{
						chat = WFormat::Action(out);
					}
					else
					{
						chat = WFormat::SendPrivMsg(GetUserID(), me, other, out);
					}
					PRINT("Printing\n");
					PrintText(chat);
				}
			}
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
					PrintError(tr("User(s) not found!"));
			}
			else if (!isping && reply)
			{
				if (fSettings->GetError())
				{
					*reply = true;
					// this will put an error message in the private window
					text = WFormat::Error(tr("User doesn't exist!"));
				}
			}
		}
		else
		{
			QString qsendtext;
			while (iter != sendTo.end())
			{
#ifdef _DEBUG
				WString wUser((*iter).second.user()->GetUserName());
				WString wText((*iter).second.text);
				PRINT("Found user %S and rest of text %S\n", wUser.getBuffer(), wText.getBuffer());
#endif

				user = (*iter).second.user;

				QString sid = user()->GetUserID();
				QString sendText = (*iter).second.text;

				if (isping)
				{
					if (fSettings->GetInfo())
					{
						QString pingMsg = WFormat::PingSent(sid, FixStringStr(user()->GetUserName())); // <postmaster@raasu.org> 20021112
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
					qsendtext = sendText;
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
						fmt = WFormat::Action(qsendtext);
					}
					else
					{
						fmt = WFormat::LocalText(GetUserID(), name, qsendtext);
					}
					text = fmt;
				}
			}
		}
	}
	else
	{
		if (fSettings->GetError())
			PrintError(tr("No users passed."));
	}
}

void
WinShareWindow::HandleMessage(MessageRef msg)
{
/* 
* The batch version of all the printing methods are used here because
* this HandleMessage() is called by HandleSignal() only, and nobody else
* should call it directly. HandleSignal() starts the batch for us if
* an MTT_EVENT_INCOMING_MESSAGE is received, and then closes the batch, which
* flushes the buffer.
*/
	switch (msg()->what)
	{
	case TTP_START_QUEUE:
		{
			const char * from;
			if (msg()->FindString(PR_NAME_SESSION, &from) == B_NO_ERROR)
			{
				QString qFrom = from;
				StartQueue(qFrom);
			}
			break;
		}
	case WDownload::TransferNotifyRejected:
		{
			const char * from;
			uint32 port;
			if (
				(msg()->FindString(PR_NAME_SESSION, &from) == B_NO_ERROR) &&
				(msg()->FindInt32("port", (int32 *) &port) == B_NO_ERROR)
				)
			{
				QString qFrom = QString::fromUtf8(from);
				uint64 timeLeft = (uint64) -1;
				(void) msg()->FindInt64("timeleft", (int64 *)&timeLeft);
				TransferCallbackRejected(qFrom, timeLeft, port);
			}
			break;
		}
		// Putting the scan here seems to fix the crash when 
		// shares are scanned on startup (during connect to server)
	case PR_RESULT_PONG:
		{
			if (fGotResults)
			{
				// Execute OnConnect commands here, when all user information should be available
				
				if ((fOnConnect != QString::null) && fOnConnect.length() > 2)
				{
					ExecCommand(fOnConnect);
					if (fOnConnect.startsWith("/search")) // only search one time, everything else should be persistent
					{
						fOnConnect = QString::null;
					}
				}
				
				if ((fOnConnect2 != QString::null) && fOnConnect2.length() > 2)
				{
					ExecCommand(fOnConnect2);
					fOnConnect = fOnConnect2;
					fOnConnect2 = QString::null;
				}
				
				// run the scan now (if needed)
				// begin the file scan thread
				PRINT("Checking...\n");
				if (fSettings->GetSharingEnabled())
				{
					StartAcceptThread();
					WaitOnFileThread(false);
					fFileScanThread->Lock();
					int numShares = fFileScanThread->GetNumFiles();
					fFileScanThread->Unlock();
					if (!fFilesScanned || (numShares == 0))
						ScanShares();
					else
						UpdateShares();
				}
			}
			else
			{
				SetSearchStatus(tr("passive"), 2);
				fGotResults = true;
			}
			break;
		}
		
		
	case NetClient::CONNECT_BACK_REQUEST:
		{
			PRINT("\tCONNECT_BACK_REQUEST\n");
			
			const char * session;
			int32 port;
			if ((msg()->FindString(PR_NAME_SESSION, &session) == B_OK) && (msg()->FindInt32("port", &port) == B_OK))
			{
				WUserRef user = fNetClient->FindUser(session);
				if (user())
				{
					OpenDownload();
					fDLWindow->AddUpload(user()->GetUserHostName(), port);
				}
			}
			break;
		}
		
	case NetClient::NEW_PICTURE:
		{
			PRINT("\tNEW_PICTURE\n");
			// QString userName = tr("Unknown");
			QString userID;
			QString file;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			{
				const char * name;			// original name of picture

				if (msg()->FindString("name", &name) == B_OK)
					file = QString::fromUtf8(name);
			}

			WUserRef user = fNetClient->FindUser(userID);
			if (user())
			{
				if (IsIgnored(user))
					return;
			}

			uint8 * data;
			size_t numBytes;

			if (msg()->FindDataPointer("picture", B_RAW_TYPE, (void **)&data, (uint32 *)&numBytes) == B_OK)
			{
				ByteBufferRef buf = GetByteBufferFromPool();
				if (buf())
				{
					if (buf()->SetNumBytes(numBytes, false) == B_OK)
					{
						buf()->SetBuffer(numBytes, data);
						uint32 myChecksum, chk;
						if (msg()->FindInt32("chk", (int32 *) &chk) == B_OK)
						{
							myChecksum = CalculateChecksum(buf()->GetBuffer(), buf()->GetNumBytes());
							if (myChecksum != chk)
								return;
						}
						SavePicture(file, buf);
						if (fPicViewer->LoadImage(file))
							fPicViewer->show();
					}
				}
			}
				
			break;
		}
	/* Tunneled transfers:
	//
	// my_id     = message sender's tunnel id
	// tunnel_id = message recipient's tunnel id
	// upload    = true, if sender thinks it's his/her upload...
	*/
	case NetClient::REQUEST_TUNNEL:
		{
			PRINT("\tREQUEST_TUNNEL\n");

			// QString userName = tr("Unknown");
			QString userID;
			int32 hisID;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			msg()->FindInt32("my_id", &hisID);

			WUserRef uref = FindUser(userID);

			if (uref())
			{
				if (IsIgnored(uref()->GetUserID(), true))
				{
					MessageRef rej(GetMessageFromPool(NetClient::REJECT_TUNNEL));
					if (rej())
					{
						QString to("/*/");
						to += userID;
						to += "/beshare";
						rej()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
						rej()->AddString(PR_NAME_SESSION, (const char *) fNetClient->LocalSessionID().utf8());
						rej()->AddInt32("tunnel_id", (int32) hisID);
						fNetClient->SendMessageToSessions(rej);
					}
				}
				else
				{
					MessageRef acc(GetMessageFromPool(NetClient::ACCEPT_TUNNEL));
					if (acc())
					{
						QString to("/*/");
						to += userID;
						to += "/beshare";
						acc()->AddString(PR_NAME_KEYS, (const char *) to.utf8());
						acc()->AddString(PR_NAME_SESSION, (const char *) fNetClient->LocalSessionID().utf8());
						void * tunnelID = NULL;
						OpenDownload();
						if (fDLWindow->CreateTunnel(userID, hisID, tunnelID))
						{
							acc()->AddInt32("tunnel_id", (int32) hisID);
							acc()->AddInt32("my_id", (int32) tunnelID);
							fNetClient->SendMessageToSessions(acc);
						}
					}
				}
			}

			break;
		}
	case NetClient::ACCEPT_TUNNEL:
		{
			PRINT("\tACCEPT_TUNNEL\n");

			// QString userName = tr("Unknown");
			QString userID;
			int32 hisID;
			int32 myID;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			msg()->FindInt32("my_id", &hisID);
			msg()->FindInt32("tunnel_id", &myID);

			WUserRef uref = FindUser(userID);

			if (uref())
			{
				if (IsIgnored(uref()->GetUserID(), true))
					return;

				OpenDownload();
				fDLWindow->TunnelAccepted(myID, hisID);
			}
			break;
		}
	case NetClient::REJECT_TUNNEL:
		{
			PRINT("\tREJECT_TUNNEL\n");

			// QString userName = tr("Unknown");
			QString userID;
			int32 myID;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			msg()->FindInt32("tunnel_id", &myID);

			WUserRef uref = FindUser(userID);

			if (uref())
			{
				if (IsIgnored(uref()->GetUserID(), true))
					return;

				if (fDLWindow)
					fDLWindow->TunnelRejected(myID);
			}
			break;
		}
	case NetClient::TUNNEL_MESSAGE:
		{
			PRINT("\tTUNNEL_MESSAGE\n");

			// QString userName = tr("Unknown");
			QString userID;
			int32 myID;
			bool upload = false;
			MessageRef tmsg;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			msg()->FindInt32("tunnel_id", &myID);

			msg()->FindBool("upload", &upload);
			msg()->FindMessage("message", tmsg);

			WUserRef uref = FindUser(userID);

			if (uref())
			{
				if (IsIgnored(uref()->GetUserID(), true))
					return;

				OpenDownload();
				fDLWindow->TunnelMessage(myID, tmsg, upload); // His/her upload is our download
			}

			break;
		}
	case NetClient::NEW_CHAT_TEXT:
		{
			QString text;		// <postmaster@raasu.org> 20021001 -- UTF-8 decoding needs this
			QString userName = tr("Unknown");
			QString userID;
			
			{
				const char * session;		// from user (their session id)

				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
					userID = QString(session);
			}

			{
				const char * strTemp;

				// <postmaster@raasu.org> 20021001 -- Convert from UTF-8 to Unicode
				if (msg()->FindString("text", &strTemp) == B_OK)
					text = QString::fromUtf8(strTemp);	
			}
			
			// get user info first
			WUserRef user = fNetClient->FindUser(userID);
			if (user())
			{
				userName = user()->GetUserName().stripWhiteSpace();
				if (!IsWhiteListed(user))
				{ 
					if (IsFilterListed(text))
					return;

					// User is repeating him/herself?
					if (user()->GetLastLine() == text)
						return;
					else
						user()->SetLastLine(text);
				}
			}
			
			// check for / commands
			if ( text.lower().left(6) == "/me's " )
			{
				if ( !IsIgnored(user) )
				{
					QString msg = GetParameterString(text);
					if ((msg.length() > 0) && fSettings->GetChat())
						Action(userName + "'s", msg);
				}
			}
			else if (text.lower().left(4) == "/me ")
			{
				if ( !IsIgnored(user) )
				{
					QString msg = GetParameterString(text);
					if ((msg.length() > 0) && fSettings->GetChat())
						Action(userName, msg);
				}
			}
			else	// regular message
			{
				QString chat;
				bool priv;
				if (msg()->FindBool("private", &priv) == B_OK)	// is it private?
				{
#if !defined(NONUKE) && defined(NDEBUG)
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

					if ( Remote(userID, text) ) 
						break;

					if (fSettings->GetPrivate())	// and are we accepting private msgs?
					{
						if ( !IsIgnored(user) )
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
									if (user()->GetUserID() == userID)
									{
										win->PutChatText(userID, text);
										foundPriv = true;
										// continue... this user may be in multiple windows... :)
									}
								}
							}
							pLock.unlock();
							if (foundPriv)
								return;
							else if ( IsAutoPrivate( userID ) )
							{
								// Create new Private Window
								WPrivateWindow * win = new WPrivateWindow(this, fNetClient, NULL);
								if (win)
								{
									WUserRef pu = FindUser(userID);
									if (pu())
									{
										// Add user to private window
										win->AddUser(pu);
										// Send text to private window
										win->PutChatText(userID, text);
										// Show newly created private window
										win->show();
										// ... and add it to list of private windows
										WPrivPair p = MakePair(win);
										pLock.lock();
										fPrivateWindows.insert(p);
										pLock.unlock();
										return;
									}
								}
							}

							PRINT("Fixing nameText\n");
							QString nameText = FixStringStr(text);
							if (nameText.startsWith(FixStringStr(userName) + " ") || nameText.startsWith(FixStringStr(userName) + "'s ")) // simulate action?
							{
								chat = WFormat::Action(nameText);
							}
							else
							{
								chat = WFormat::ReceivePrivMsg(userID, FixStringStr(userName), nameText);
							}
							if (fSettings->GetSounds())
								QApplication::beep();
#ifdef WIN32
							if (fWinHandle && !this->isActiveWindow() && (fSettings->GetFlash() & WSettings::FlashPriv))	// if we have a valid handle AND we are not active AND the user wants to flash
							{
								WFlashWindow(fWinHandle);
							}
#endif // WIN32
							
							TextEvent(this, chat, WTextEvent::ChatTextEvent); 
						}
					}
				}
				else
				{
					if (fSettings->GetChat())
					{
						if ( !IsIgnored(user) )
						{
							PRINT("Fixing string\n");
							QString nameText = FixStringStr(text);
							PRINT("Name said\n");
							if (NameSaid(nameText) && fSettings->GetSounds())
								QApplication::beep();
							if (MatchUserFilter(user, fWatch))
								chat += WFormat::RemoteWatch(userID, FixStringStr(userName), nameText);
							else
								chat += WFormat::RemoteText(userID, FixStringStr(userName), nameText);
							TextEvent(this, chat, WTextEvent::ChatTextEvent); 
						}
					}
				}
			}
			break;
		}
		case NetClient::ChannelText:
			{
				QString text;		// <postmaster@raasu.org> 20021001 -- UTF-8 decoding needs this
				QString channel;
				QString userID;

				{
					const char * session;		// from user (their session id)
					const char * strTemp;
					const char * strChannel;
			
					msg()->FindString(PR_NAME_SESSION, &session);
					msg()->FindString("text", &strTemp);
					msg()->FindString("channel", &strChannel);
			
					// <postmaster@raasu.org> 20021001 -- Convert from UTF-8 to Latin-1
					text = QString::fromUtf8(strTemp);
					channel = QString::fromUtf8(strChannel);
					userID = QString(session);
				}
			
				// get user info first
				WUserRef user = fNetClient->FindUser(userID);
				if (user())
				{
					// User is repeating him/herself?

					if (!IsWhiteListed(user))
					{
						if (user()->GetLastLine(channel) == text)
							return;
						else
							user()->SetLastLine(channel, text);
					}

					emit NewChannelText(channel, userID, text);
				}

				break;
			}
		case NetClient::ClientConnected:
			{
				String repto;
				String repname;
				int64 rtime;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindInt64("registertime", &rtime) == B_OK) &&
					(msg()->FindString("name", repname) == B_OK)
					)
				{
					if (rtime >= GetRegisterTime())
					{
						MessageRef col(GetMessageFromPool(NetClient::RegisterFail));
						String to("/*/");
						to += repto;
						to += "/unishare";
						col()->AddString(PR_NAME_KEYS, to);
						col()->AddString("name", (const char *) GetUserName().utf8() );
						col()->AddString(PR_NAME_SESSION, (const char *) GetUserID().utf8());
						col()->AddInt64("registertime", GetRegisterTime() );
						
						fNetClient->SendMessageToSessions(col);
					}
				}
				break;
			}

		case NetClient::RegisterFail:
			{
				String repto;
				int64 rtime;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) && 
					(msg()->FindInt64("registertime", &rtime) == B_OK)
					)
				{
					if (rtime <= GetRegisterTime())
					{
						PrintError( tr("Nick collision with user #%1").arg(repto.Cstr()));
					}
				}
				break;
			}
		case NetClient::ChannelCreated:
			{
				String channel, repto;
				int64 rtime;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindString("channel", channel) == B_OK) &&
					(msg()->FindInt64("when", &rtime) == B_OK)
					)
				{
					QString qOwner = QString::fromUtf8(repto.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelCreated(qChan, qOwner, rtime);
				}

				break;
			}
		case NetClient::ChannelJoin:
			{
				String channel, repto;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) && 
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelJoin(qChan, qUser);
				}
				break;
			}
		case NetClient::ChannelPart:
			{
				String channel, repto;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) && 
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelPart(qChan, qUser);
				}
				break;
			}
		case NetClient::ChannelInvite:
			{
				String channel, repto, who;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindString("who", who) == B_OK) &&
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qWho = QString::fromUtf8(who.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelInvite(qChan, qUser, qWho);
				}
				break;
			}
		case NetClient::ChannelKick:
			{
				String channel, repto, who;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindString("who", who) == B_OK) &&
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qWho = QString::fromUtf8(who.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelKick(qChan, qUser, qWho);
				}
				break;
			}
		case NetClient::ChannelSetTopic:
			{
				String channel, repto, topic;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindString("topic", topic) == B_OK) && 
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qTopic = QString::fromUtf8(topic.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelTopic(qChan, qUser, qTopic);
				}
				break;
			}
		case NetClient::ChannelSetPublic:
			{
				String channel, repto;
				bool pub;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) &&
					(msg()->FindBool("public", &pub) == B_OK) &&
					(msg()->FindString("channel", channel) == B_OK)
					)
				{
					QString qUser = QString::fromUtf8(repto.Cstr());
					QString qChan = QString::fromUtf8(channel.Cstr());
					ChannelPublic(qChan, qUser, pub);
				}
				break;
			}
		case NetClient::PING:
			{
				String repto;
				if (msg()->FindString(PR_NAME_SESSION, repto) == B_OK)
				{
					WUserIter uit = fNetClient->Users().find(QString::fromUtf8(repto.Cstr()));
					if (uit != fNetClient->Users().end())
					{
						if (fSettings->GetInfo())
						{
							PRINT("Print ping\n");
							WUserRef user = (*uit).second;
							QString system = WFormat::GotPinged(repto.Cstr(), FixStringStr(user()->GetUserName())); // <postmaster@raasu.org> 20021112
							PrintSystem(system);
						}
					}
					msg()->what = NetClient::PONG;
					
					String tostr("/*/");
					tostr += repto;
					tostr += "/beshare";
					
					msg()->RemoveName(PR_NAME_KEYS);
					msg()->AddString(PR_NAME_KEYS, tostr);
					msg()->RemoveName(PR_NAME_SESSION);
					msg()->AddString(PR_NAME_SESSION, (const char *) GetUserID().utf8());
					msg()->RemoveName("version");
					
					QString version = tr("Unizone (English)");
					version += " ";
#if defined(__LINUX__) || defined(linux)
					version += "(Linux) ";
#elif defined(__FreeBSD__)
					version += "(FreeBSD) ";
#endif
					version += WinShareVersionString();
					
					// <postmaster@raasu.org> 20021025 -- Added uptime calculating for Windows
					// <postmaster@raasu.org> 20021231 -- and for Linux ;)
					int64 fUptime = GetUptime();
					int64 fOnlineTime = GetCurrentTime64() - fLoginTime;
					msg()->AddString("version", (const char *) version.utf8());
					msg()->RemoveName("uptime");
					msg()->AddInt64("uptime", fUptime);
					msg()->RemoveName("onlinetime");
					msg()->AddInt64("onlinetime", fOnlineTime);
					
					fNetClient->SendMessageToSessions(msg);
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
				
				if ((msg()->FindString(PR_NAME_SESSION, &session) == B_OK) && (msg()->FindInt64("when", &when) == B_OK))
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
							QString pong = WFormat::RemoteName(session, FixStringStr(user()->GetUserName()));
							int32 time = ((GetCurrentTime64() - when) / 10000L);
							QString versionString = GetRemoteVersionString(msg);
							
							pong += WFormat::PingText(time, versionString);
							
							int64 uptime, onlinetime;
							if ((msg()->FindInt64("uptime", &uptime) == B_OK) && (msg()->FindInt64("onlinetime", &onlinetime) == B_OK))
							{
								pong += " ";
								pong += WFormat::PingUptime(MakeHumanTime(uptime), MakeHumanTime(onlinetime));
							}
							
							PrintText(pong);
						}
					}
				}
				break;
			}
		case TimeRequest:
			{
				String lt, session;
				if (msg()->FindString(PR_NAME_SESSION, session) == B_OK)
				{
					String tostr("/*/");
					tostr += session;
					tostr += "/unishare";

					time_t currentTime = time(NULL);
					tm * myTime;
					char zone[64];
					
					bool g = false;
					(void) msg()->FindBool("gmt",&g);

					if (g)
						myTime = gmtime(&currentTime);
					else
						myTime = localtime(&currentTime);
					
					lt = asctime(myTime);
					lt = lt.Substring(0, "\n");
					
					if (g)
						strcpy(zone, "GMT");
					else if (myTime->tm_isdst != 1) 
						strcpy(zone, tzname[0]);
					else 
						strcpy(zone, tzname[1]);

					MessageRef tire(GetMessageFromPool(TimeReply));
					if (tire())
					{
						// Make sure we send using UTF-8 and not local multi-byte encoding
						QString qlt = QString::fromLocal8Bit(lt.Cstr());
						QString qzone = QString::fromLocal8Bit(zone);
						tire()->AddString(PR_NAME_KEYS, tostr);
						tire()->AddString(PR_NAME_SESSION, (const char *) GetUserID().utf8());
						tire()->AddString("time", (const char *) qlt.utf8());
						tire()->AddString("zone", (const char *) qzone.utf8());
						fNetClient->SendMessageToSessions(tire);
					}
				}
				break;
			}
		case TimeReply: // Reply to TimeRequest
			{
				String sTime, sZone;
				const char * session;
				if (msg()->FindString(PR_NAME_SESSION, &session) == B_OK)
				{
					WUserRef user = fNetClient->FindUser(session);
					if (user())
					{
						if ((msg()->FindString("time", sTime) == B_OK) && (msg()->FindString("zone", sZone) == B_OK))
						{
							QString userName = user()->GetUserName().stripWhiteSpace();
							QString qstamp = tr("Current time: %1 %2").arg(QString::fromUtf8(sTime.Cstr())).arg(QString::fromUtf8(sZone.Cstr()));
							QString qTime = WFormat::RemoteText(session, FixStringStr(userName), qstamp);
							PrintText(qTime);
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
	fGotResults = true;
	if (fNetClient)
	{
		WaitOnFileThread(false);	// make sure our scan thread is dead
		Disconnect();
		fNetClient->SetUserName(GetUserName()); // We need this for binkies

		fConnectTimer->start(60000, true); // 1 minute
		
		if (fNetClient->Connect(fServer) == B_OK)
		{
			if (fSettings->GetInfo())
				PrintSystem(tr("Connecting to server %1.").arg(fServer));
		}
		else
		{
			if (fSettings->GetInfo())
				PrintSystem(tr("Connection to server failed!"));
		}
	}
	fLoginTime = GetCurrentTime64();
}

void
WinShareWindow::Connect(const QString & server)
{
	fServer = server;
	if (fServerList->currentText() != fServer)
	{
		GotUpdateCmd("addserver", fServer);
		for (int i = 0; i < fServerList->count(); i++)
		{
			QString slist = fServerList->text(i).stripWhiteSpace();
			if (slist.lower() == fServer.lower())
			{
				// found our server
				fServerList->setCurrentItem(i);
			}
		}
	}
	Connect();
}

void
WinShareWindow::Disconnect()
{
	if (fReconnectTimer->isActive())
	{
		PrintSystem(tr("Reconnect timer stopped"));
		fReconnectTimer->stop();
	}
	fDisconnectFlag = true; // User disconnection

	Disconnect2();
}

void
WinShareWindow::Disconnect2()
{
	if (fConnectTimer->isActive())
		fConnectTimer->stop();

	WaitOnFileThread(false);

	if (fNetClient && fNetClient->IsConnected()	/* this is to stop a forever loop involving the DisconnectedFromServer() signal */)
	{
		fNetClient->Disconnect();
	}
	fUpdateThread->Disconnect();
	// just in case..
	fServerThread->Disconnect();
	
}

void
WinShareWindow::ShowHelp(const QString & command)
{
	QString temp;
	QString qNone = tr("&lt;None&gt;");

	QString helpText	=	"\n";
	helpText			+=	tr("Unizone Command Reference");
	helpText			+=	"\n";
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/action [action] - do something");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/addautopriv [pattern] - update the auto-private pattern (can be a user name, or several names, or regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/addblacklist [pattern] - update the blacklist pattern (can be a user name, or several names, or regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=  tr("/addfilter [pattern] - update the word filter pattern");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=  tr("/addwhitelist [pattern] - update the whitelist pattern (can be a user name, or several names, or regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/addignore [pattern] - update the ignore pattern (can be a user name, or several names, or a regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/adduser [name or session ids] - add users to a private chat window (works in private windows only!)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/autopriv [pattern] - set the auto-private pattern (can be a user name, or several names, or regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/away - set away state (same as selecting away from the list)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/awaymsg - away message for away state (when /away is invoked)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/blacklist [pattern] - set the blacklist pattern (can be a user name, or several names, or a regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/btime [gmt] - Broadcast and show local (or GMT) time");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/buptime - Broadcast and show uptime");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/chkserver [index] - check server string");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/chkstatus [index] - check status string");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/chkuser [index] - check nick string");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/clear - clear the text in the chat view");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/clearline - clear all the line buffers");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/clearresumes - clear all pending resumes");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/clearstats - clear transfer statistics");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/compression [level] - set or view message compression level");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/connect - connect to the currently selected server");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/disconnect - disconnect from server");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/dns [user|host] - give information about host");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=  tr("/filter [pattern] - set the word filter pattern");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/help [command] - show help for command (no '/' in front of command) or show this help text if no command given.");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/heremsg - message for here state");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/ignore [pattern] - set the ignore pattern (can be a user name, or several names, or a regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/ip [user|host] - give information about host");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/logged - show the time you have been logged in to a server");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/me [action] - /action synonym");
	helpText			+=	"\n\t\t\t\t"; 
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
	helpText			+=	tr("/memory - show number of bytes Unizone is using memory");
	helpText			+=	"\n\t\t\t\t"; 
#endif
	helpText			+=	tr("/msg [name] [message] - send a private message");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/nick [name] - change your user name");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/onconnect [command] - set or clear command to perform on successful connect");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/picture [name or session ids] - send picture to other clients");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/ping [name or session ids] - ping other clients");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/priv [name or session ids] - open private chat with these users added");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/quit - quit Unizone");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/remote [password] - set & view remote password");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/removeuser [name or session ids] - remove users from a private chat window (works in private windows only!)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/remserver [index] - remove server from server list");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/remstatus [index] - remove status from status list");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/remuser [index] - remove nick from nick list");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/resumes - list files waiting to be resumed");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/reverse [text] - say text in reverse");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/revsay [nick]: [text] - say text reversed but prefix with nick");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/running - show time elapsed since you started Unizone");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/save - saves settings (might be necessary after editing drop-down lists)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/scan - rescan shared directory");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/search [pattern] - open search window");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/server [server] - set the current server");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/serverinfo - check status of server");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/shell [command] - execute command");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/showpatterns - show auto-private, blacklist, ignore and watch patterns");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/showstats - show transfer statistics");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/status [status] - set status string");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/time [gmt] - show local (or GMT) time");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/time [nick] [gmt] - request time stamp from other user");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/unautopriv [name] - remove name from auto-private list");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/unblacklist [name] - remove name from blacklist");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=  tr("/unfilter [pattern] - remove pattern from word filters");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/unignore [name] - remove name from ignore list");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/unwhitelist [name] - remove name from whitelist");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/uptime - show system uptime");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/users - show number of users connected");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/version - show client version strings");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/view - view picture on local machine");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/watch [pattern] - set the watch pattern (can be a user name, or several names, or a regular expression)");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=  tr("/whitelist [pattern] - set the whitelist pattern");
	helpText			+=	"\n\n";
	helpText			+=	tr("Admin Command Reference");
	helpText			+=	"\n";
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/addbans [users] - add bans by user names or session ids");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/addipbans [ips] - add bans by ip addresses");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/kick [users] - kick by user names or session ids");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/kickips [ips] - kick by ip addresses");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/rembans [users] - remove bans by user names");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/remipbans [ips] - remove bans by ip addresses");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/require [ips] - add require mask");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/unrequire [ips] - remove require mask");
#if defined(BETA) || defined(_DEBUG)
	helpText			+=	"\n\n\t\t\t\t"; 
	helpText			+=	tr("The list of commands is being worked on. More will be added");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("as time goes on.");
#endif
	helpText			+=	"\n";
	helpText			+=	"\n"; 

	temp = CheckIfEmpty(fAutoPriv, qNone);
	helpText			+=	tr("Auto-private pattern: %1").arg(temp);
	helpText			+=	"\n"; 
	
	temp = CheckIfEmpty(fBlackList, qNone);
	helpText			+=	tr("Blacklist pattern: %1").arg(temp);
	helpText			+=	"\n"; 

	temp = CheckIfEmpty(fIgnore, qNone);
	helpText			+=	tr("Ignore pattern: %1").arg(temp);
	helpText			+=	"\n"; 

	temp = CheckIfEmpty(fWatch, qNone);
	helpText			+=	tr("Watch pattern: %1").arg(temp);
	helpText			+=	"\n"; 

	if (fOnConnect.isEmpty())
	{
		helpText			+=	tr("On Connect: Do Nothing ;)");
		helpText			+=	"\n"; 
	}
	else
	{
		helpText			+=	tr("On Connect:");
		helpText			+=	"\n"; 
		helpText			+=	tr("1. %1").arg(fOnConnect);
		if (!fOnConnect2.isEmpty())
		{
			helpText			+=	"\n"; 
			helpText			+=	tr("2. %1").arg(fOnConnect2);
		}
	}

	QString str;
	if (command.isEmpty())
	{
		str = ParseStringStr(helpText);
		PrintSystem(str);
	}
	else
	{
		QString cmd = "\t/" + command + " ";
		int s = 0;
		bool found = false;
		while (true)
		{
			int i = helpText.find(cmd,s);
			if (i >= 0)
			{
				if (s == 0)		// First one?
				{
					str = "\n"; 
					str += tr("Help for %1:").arg(command);
					str += "\n";
				}
				i++;
				QString chelp = helpText.mid(i);
				int j = chelp.find("\n", 5);
				if (j >= 0)
				{
					found = true;
					
					chelp.truncate(j);
										
					str += "\n";
					str += chelp;
				}
			}
			else
			{
				break;
			}
			s = i;
		}
		if (found)
		{
			ParseString(str);
			PrintSystem(str);
		}
		else
		{
			PrintError(tr("Command %1 not found").arg(command));
		}
	}
}


bool
WinShareWindow::IsConnected(const QString & user)
{
	// Find the user record
	//

	WUserRef uref = FindUser(user);

	// Valid user reference?
	//

	if (uref() != NULL)	
	{
		// Valid reference!
	
		return true;
	}
	else
	{
		// Invalid reference!

		return false;
	}
}


WUserRef
WinShareWindow::FindUser(const QString & user)
{
	WUserMap & umap = fNetClient->Users();
	for (WUserIter iter = umap.begin(); iter != umap.end(); iter++)
	{
		if (MatchUserFilter((*iter).second, user))
		{
			return (*iter).second;
		}
	}
	return WUserRef(NULL, NULL);
}

WUserRef 
WinShareWindow::FindUserByIPandPort(const QString & ip, uint32 port)
{
	if (fNetClient)
	{
		return fNetClient->FindUserByIPandPort(ip, port);
	}
	else
	{
		return WUserRef(NULL, NULL);
	}
}


bool
WinShareWindow::Remote(const QString & /* session */, const QString &text)
{
	QString qItem;
	if (!text.startsWith("!remote"))	// Is a remote request?
		return false;
	if (fRemote.isEmpty())					// is remote control enabled?
		return false;

	QString cmd = text.mid(8);

	// try to parse password...
	int sp = cmd.find("\n");
	
	if (sp == -1) 
		return false;

	QString pass = cmd.left(sp);
	
	// Correct password?
	if (pass != fRemote)
		return false;

	// Parse for commands...
	QStringTokenizer qTok(cmd.mid(sp+1),"\n");
	while ((qItem = qTok.GetNextToken()) != QString::null)
	{
		if (!qItem.lower().startsWith("/shell"))
		{
			ExecCommand(qItem);
		}
	}
	return true;
}

// List files waiting to be resumed

void 
WinShareWindow::ListResumes()
{
	QString out;

	rLock.lock();
	WResumeIter it = fResumeMap.begin();
	out = "\n" + tr("Resume list:");
	int i = 0;
	while (it != fResumeMap.end())
	{
		if ((*it).first != QString::null)
		{
			out += "\n" + tr("File %1: (%2) from %3").arg(i).arg((*it).second.fRemoteName).arg((*it).first);
			if ((*it).second.fLocalName != QString::null)
			{
				out += "\n" + tr("- Local File: %1").arg((*it).second.fLocalName);
			}
			i++;
		}
		it++;
	}
	out += "\n" + tr("Total: %1 files").arg(i);
	FixString(out);
	rLock.unlock();

	PrintSystem(out);
}

void
WinShareWindow::ClearResumes()
{
	rLock.lock();
	fResumeMap.clear();
	PrintSystem(tr("Cleared resume list."));
	rLock.unlock();
}

void
WinShareWindow::PrintAddressInfo(const WUserRef & user, bool verbose)
{
	QString addr, uname, uid;
	QString out("");
	uint32 address = 0;
	char host[16];
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure

	if (user() != NULL)
	{
		address = GetHostByName(user()->GetUserHostName());
		addr = user()->GetUserHostName();
		uname = user()->GetUserName();
		uid = user()->GetUserID();
	}

	if (address > 0)
	{
		Inet_NtoA(address, host);
					
		out += "\n" + tr("Address info for user #%1:").arg(uid);
		out += "\n" + tr("User Name: %1").arg(uname);
					
		out += "\n" + tr("IP Address: %1").arg(host);
		
		if (verbose)
		{
			if (user()->GetPort() != 0)
				out += "\n" + tr("Port: %1").arg( user()->GetPort() );
					
			iaHost.s_addr = inet_addr(host);
			lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
					
			if (lpHostEntry != NULL)
			{
				out += "\n" + tr("Host Name: %1").arg(lpHostEntry->h_name);
			}
		}
		FixString(out);
		PrintSystem(out);
	}					
	else if (fSettings->GetError())
	{
		PrintError(tr("No address info for %1").arg(uname));
	}
}

bool
WinShareWindow::PrintAddressInfo(uint32 address, bool verbose)
{
	char host[16];
	struct in_addr iaHost;	   // Internet address structure
	LPHOSTENT lpHostEntry;	   // Pointer to host entry structure
	QString out("");
	bool found = false;

	if (address > 0)
	{
		Inet_NtoA(address, host);
					
		if (!verbose)
		{
			out += tr("IP Address: %1").arg(host);
			found = true;
		}
		else
		{
			out += "\n" + tr("Address info for %1:").arg(host);
				
			iaHost.s_addr = inet_addr(host);
			lpHostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
					
			if (lpHostEntry != NULL)
			{
				out += "\n" + tr("Host Name: %1").arg(lpHostEntry->h_name);
				found = true;
			}
					
			// List all users from this ip
						
			WUserMap cmap;
			fNetClient->FindUsersByIP(cmap, host);
			if (!cmap.empty())
			{
				out += "\n" + tr("Connected users:");

				for (WUserIter it = cmap.begin(); it != cmap.end(); it++)
				{
					if ( (*it).second() )
					{
						QString uid = (*it).second()->GetUserID();
						QString uname = (*it).second()->GetUserName();
						uint32 port = (*it).second()->GetPort();
						if (port != 0)
							out += "\n" + tr("#%1 - %2 (port: %3)").arg(uid).arg(uname).arg(port);
						else
							out += "\n" + tr("#%1 - %2").arg(uid).arg(uname);
						found = true;
					}
				}
			}
		}
		if (found)
		{
			FixString(out);
			PrintSystem(out);
		}
	}
	return found;
}

void
WinShareWindow::GetAddressInfo(const QString & user, bool verbose)
{
	int numMatches;
	WUserMap wmap;
	numMatches = FillUserMap(user, wmap);
	uint32 address = 0;
	if (numMatches > 0)	// Found atleast one match in users
	{
		WUserIter uiter = wmap.begin();
		while (uiter != wmap.end())
		{
			PrintAddressInfo((*uiter).second, verbose);
			uiter++;
		}
	}
	else				// Try as hostname or ip address
	{
		address = GetHostByName(user);
		if (!PrintAddressInfo(address, verbose) && fSettings->GetError())
		{
			PrintError(tr("No address info for %1").arg(user));
		}
	}				
}

void
WinShareWindow::ExecCommand(const QString &command)
{
	WTextEvent * wte = new WTextEvent(command);
	if (wte)
	{
		SendChatText(wte, false);
		delete wte;
	}
}

void
WinShareWindow::TransferCallbackRejected(const QString &qFrom, int64 timeLeft, uint32 port)
{
	if (!fDLWindow)
		return;
	fDLWindow->TransferCallBackRejected(qFrom, timeLeft, port);
}

void
WinShareWindow::SendRejectedNotification(MessageRef rej)
{
	fNetClient->SendMessageToSessions(rej);
}

void
WinShareWindow::ConnectionAccepted(SocketHolderRef socketRef)
{
	PRINT("\tWinShareWindow::ConnectionAccepted\n");
	int socket = socketRef() ? (socketRef()->ReleaseSocket()) : -1;
	uint32 ip;
	if (socket >= 0 && (ip = GetPeerIPAddress(socket)) > 0)
	{
		OpenDownload();
		fDLWindow->AddUpload(socket, ip, false);
	}
}

QString
WinShareWindow::GetUserID() const
{ 
	if (fNetClient)
	{
		if (fNetClient->IsConnected())
			return fNetClient->LocalSessionID();
	}
	return QString::null;
}

void
WinShareWindow::UpdateUserCount()
{
	if (fNetClient->IsConnected())
	{
		int n = fUsers->childCount() + 1;
		fStatusBar->setText(tr( "Number of users logged in: %1" ).arg(n), 0);
	}
	else
		fStatusBar->setText(tr( "Not connected." ), 0);
}

