// An extension of the main WinShare class
// Contains all the networking related methods

#include <qapplication.h>
#include <q3filedialog.h>
#include <qregexp.h>
#include <QDesktopWidget>
#include <QPixmap>

#include "winsharewindow.h"
#include "global.h"
#include "version.h"
#include "settings.h"
#include "debugimpl.h"
#include "formatting.h"
#include "platform.h"
#include "picviewerimpl.h"
#include "util.h"
#include "wstring.h"
#include "wcrypt.h"
#include "gotourl.h"			// <postmaster@raasu.org> 20021116 -- for /shell
#include "tokenizer.h"			// <postmaster@raasu.org> 20030902 -- for !remote
#include "ulistview.h"
#include "downloadimpl.h"
#include "uploadimpl.h"
#include "wtransfer.h"
#include "filethread.h"
#include "netclient.h"
#include "serverclient.h"
#include "updateclient.h"
#include "wstatusbar.h"
#include "resolverthread.h"

#include "events.h"
#include "chatevent.h"
#include "textevent.h"
#include "wmessageevent.h"
#include "wpwevent.h"

#include "util/TimeUtilityFunctions.h"
#include "util/StringTokenizer.h"
#include "iogateway/PlainTextMessageIOGateway.h"
#include "iogateway/MessageIOGateway.h"
#ifdef MUSCLE_ENABLE_MEMORY_TRACKING
#include "system/GlobalMemoryAllocator.h"
#endif
#include "system/SystemInfo.h"

#include <time.h>				//                                 -- for /time

void
WinShareWindow::SendChatText(WTextEvent * e, bool * reply)
{
	if (e)	// incase the dynamic cast fails
	{
		PRINT("WinShareWindow: Received Valid WTextEvent\n");
		QString sendText = e->Text().stripWhiteSpace();

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
					if (fFileScanThread->IsInternalThreadRunning())
					{
						if (fSettings->GetError())
							PrintError(tr("Already scanning!"));
					}
					else
					{
						StartAcceptThread();
						ScanShares(true);
					}
				}
				else if (fSettings->GetError())
				{
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
			SendPingOrMsg(sendText, false, reply, e->Encrypted());
			if (reply && *reply)		// is reply wanted? and should it be sent?
				e->SetText(sendText);	// modified by SendPingOrMsg();
		}
		else if (CompareCommand(sendText, "/emsg"))
		{
			SendPingOrMsg(sendText, false, reply, true);
			if (reply && *reply)		// is reply wanted? and should it be sent?
				e->SetText(sendText);	// modified by SendPingOrMsg();
		}
		else if (CompareCommand(sendText, "/nick"))
		{
			// user name change
			QString name = GetParameterString(sendText);
			// <postmaster@raasu.org> 20021001
			if (name.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No nickname passed."));
			}
			else
			{
				if (BinkyCheck(name))
				{
					if (fSettings->GetError())
						PrintError(tr("Invalid nickname!"));
					return;
				}
				if (name.find('\n') >= 0)
				{
					if (fSettings->GetError())
						PrintError(tr("Invalid nickname!"));
					return;
				}
				NameChanged(name);
			}
		}
		else if (	
					(CompareCommand(sendText, "/me's")) || 
					(CompareCommand(sendText, "/action's")) 
				)
		{
			QString msg = GetParameterString(sendText);
			
			if (msg.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No message to send."));
			}
			else
			{
				QString send = "/me's ";
				send += msg;

				fNetClient->SendChatText("*", send);
				if (fSettings->GetChat())
					Action(GetUserName() + "'s", msg);
            }
		}
		else if (	
					(CompareCommand(sendText, "/me")) || 
					(CompareCommand(sendText, "/action")) 
				)
		{
			QString msg = GetParameterString(sendText);

			if (msg.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No message to send."));
			}
			else
			{
				QString send = "/me ";	// we don't want /action sent
				send += msg;

				fNetClient->SendChatText("*", send);
				if (fSettings->GetChat())
					Action(GetUserName(), msg);
			}
		}
		else if (sendText.startsWith("//"))	// used so that / commands can be printed
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
			pLock.Lock();
			for (unsigned int i = 0; i < fPrivateWindows.GetNumItems(); i++)
				fPrivateWindows[i]->ClearBuffer();
			pLock.Unlock();
		}
		else if (CompareCommand(sendText, "/quit"))
		{
			Exit();
		}
		else if (CompareCommand(sendText, "/status"))
		{
			// change our status
			QString newStatus = GetParameterString(sendText);

			if (!newStatus.isEmpty())
			{
				QString ns = newStatus.stripWhiteSpace();

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

			if (!server.isEmpty())
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
			if (enc.isEmpty())
			{
				if (fSettings->GetInfo())
					PrintSystem(tr("Current compression: %1").arg(_en - MUSCLE_MESSAGE_ENCODING_DEFAULT));
			}
			else
			{
				bool b;
				_en = enc.toULong(&b);
				if (b)
				{
					setStatus(tr( "Current compression: %1" ).arg(_en), 1);

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
		}
		else if (CompareCommand(sendText, "/disconnect"))
		{
			Disconnect();
		}
		else if (CompareCommand(sendText, "/awaymsg"))
		{
			QString msg = GetParameterString(sendText);

			if (!msg.isEmpty())
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

			if (!msg.isEmpty())
			{
				fHereMsg = msg;
				if (fSettings->GetInfo())
					PrintSystem(tr("Here message set to %1.").arg(fHereMsg));
			}
		}
		else if (CompareCommand(sendText, "/clear"))
		{
			Clear();	// empty the text
		}
		else if (CompareCommand(sendText, "/watch"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
				SetWatchPattern(QString::null);	// clear watch pattern
			else
				SetWatchPattern(users);
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
				PrintSystem( tr("Logged In: %1").arg(MakeHumanTime(GetCurrentTime64() - fNetClient->LoginTime())) );
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
			// <postmaster@raasu.org> 20021005 -- I think it's 1 for current user that needs to be added to get the total count
			uint iUsers = fUsers->childCount() + 1;  
			if (iUsers > fMaxUsers)
				fMaxUsers = iUsers;
			QString qUsers = QString::number(iUsers) + " (" + QString::number(fMaxUsers) + ")";
			PrintSystem(tr("Number of users logged in: %1").arg(qUsers));
		}
		else if (CompareCommand(sendText, "/priv"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
				LaunchPrivate(users);
		}
		else if (CompareCommand(sendText, "/kick"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else 
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_KICK));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapUsersToIDs(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/kickips"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_KICK));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapIPsToNodes(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/addbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDBANS));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapUsersToIDs(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/addipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDBANS));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapIPsToNodes(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/rembans"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEBANS));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapUsersToIDs(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/remipbans"))
		{
			QString users = GetParameterString(sendText);

			if (users.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEBANS));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapIPsToNodes(users));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/require"))
		{
			QString mask = GetParameterString(sendText);

			if (!mask.isEmpty())
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_ADDREQUIRES));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapIPsToNodes(mask));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		else if (CompareCommand(sendText, "/unrequire"))
		{
			QString mask = GetParameterString(sendText);

			if (!mask.isEmpty())
			{
				MessageRef mrf(GetMessageFromPool(PR_COMMAND_REMOVEREQUIRES));
				AddStringToMessage(mrf, PR_NAME_KEYS, MapIPsToNodes(mask));
				fNetClient->SendMessageToSessions(mrf);
			}
		}
		// <postmaster@raasu.org> 20021116
		else if (CompareCommand(sendText, "/shell"))
		{
			QString command = GetParameterString(sendText);

			if (!command.isEmpty())
			{
				RunCommand(command);
			}
		}
		else if (CompareCommand(sendText, "/wiki"))
		{
			QString address("http://en.wikipedia.org/wiki/");
			QString page = GetParameterString(sendText);

			if (!page.isEmpty())
			{
				page = WikiEscape(page);
				address += page;
				RunCommand(address);
			}
		}
		else if (CompareCommand(sendText, "/google"))
		{
			QString address("http://www.google.com/search?q=");
			QString page = GetParameterString(sendText);

			if (!page.isEmpty())
			{
				page = URLEscape(page);
				address += page;
				RunCommand(address);
			}
		}
		else if (CompareCommand(sendText, "/time"))
		{
			QString command = GetParameterString(sendText).lower();
			String lt;
			time_t currentTime = time(NULL);
			tm myTime;
			char zone[64];
			QString tuser = QString::null;

			if (!command.isEmpty() && (command != "gmt")) // User name?
			{
				int rpos;
				if (command.startsWith("'"))
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
					rpos = command.findRev(" ");
					if (rpos == -1)
					{
						tuser = command;
						command = QString::null;
					}
					else
					{
						if (command.mid(rpos + 1) == "gmt")
						{
							tuser = command.left(rpos);
							command = "gmt";
						}
						else
						{
							tuser = command;
							command = QString::null;
						}
					}
				}
			}

			if (tuser == QString::null)
			{
				if (command != "gmt")
					localtime_r(&currentTime, &myTime);
				else
					gmtime_r(&currentTime, &myTime);
				
				lt = asctime(&myTime);
				lt = lt.Substring(0, "\n");
				
				if (command == "gmt")
					strcpy(zone, "GMT");
				else if (myTime.tm_isdst != 1) 
					strcpy(zone, tzname[0]);
				else 
					strcpy(zone, tzname[1]);
				
				PrintSystem(tr("Current time: %1 %2").arg(QString::fromLocal8Bit(lt.Cstr())).arg(QString::fromLocal8Bit(zone)));
			}
			else
			{
				WUserMap umap;
				int uc = FillUserMap(tuser, umap);
				if (uc > 0)
				{
					WUserIter iter = umap.GetIterator(HTIT_FLAG_NOREGISTER);
					while (iter.HasMoreValues())
					{
						WUserRef tu;
						iter.GetNextValue(tu);
						QString to("/*/");
						to += tu()->GetUserID();
						to += "/unishare";

						MessageRef tire(GetMessageFromPool(TimeRequest));
						if (tire())
						{
							AddStringToMessage(tire, PR_NAME_KEYS, to);
							AddStringToMessage(tire, PR_NAME_SESSION, GetUserID());
							if (command == "gmt")
							{
								tire()->AddBool("gmt", true);
							}
							if (fSettings->GetInfo())
							{
								QString treqMsg = WFormat::TimeRequest(tu()->GetUserID(), FixString(tu()->GetUserName()));
								PrintSystem(treqMsg);
							}
						
							fNetClient->SendMessageToSessions(tire);
						}		
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
			tm myTime;
			char zone[64];

			if (command != "gmt")
				localtime_r(&currentTime, &myTime);
			else
				gmtime_r(&currentTime, &myTime);

			lt = asctime(&myTime);
			lt = lt.Substring(0, "\n");

			if (command == "gmt")
				strcpy(zone, "GMT");
			else if (myTime.tm_isdst != 1) 
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
					PrintSystem(tr("Whitelist pattern set to: %1").arg(fWhiteList));
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
		else if (CompareCommand(sendText, "/redirect"))
		{
			fPMRedirect = GetParameterString(sendText);
			if (fSettings->GetInfo())
			{
				if (fPMRedirect.isEmpty())
					PrintSystem(tr("Private Message redirect pattern cleared."));
				else
					PrintSystem(tr("Private Message redirect pattern set to: %1").arg(fPMRedirect));
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
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fUserList->count()))
					PrintSystem( tr("User %1: %2").arg(u).arg(fUserList->text(u)) );
				else if (fSettings->GetError())
					PrintError(tr("Invalid index."));
			}
		}
		else if (CompareCommand(sendText, "/remuser"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
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
		}
		else if (CompareCommand(sendText, "/chkstatus"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No index specified."));
			}
			else
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fStatusList->count()))
					PrintSystem( tr("Status %1: %2").arg(u).arg(fStatusList->text(u)) );
				else if (fSettings->GetError())
					PrintError(tr("Invalid index."));
			}
		}
		else if (CompareCommand(sendText, "/remstatus"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No users passed."));
			}
			else
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
		}
		else if (CompareCommand(sendText, "/chkserver"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No index specified."));
			}
			else
			{
				bool ok;
				int u = user.toInt(&ok);
				if (ok && (u < fServerList->count()))
					PrintSystem( tr("Server %1: %2").arg(u).arg(fServerList->text(u)) );
				else if (fSettings->GetError())
					PrintError(tr("Invalid index."));
			}
		}
		else if (CompareCommand(sendText, "/remserver"))
		{
			QString user = GetParameterString(sendText);
			
			if (user.isEmpty())
			{
				if (fSettings->GetError())
					PrintError(tr("No index specified."));
			}
			else
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
		}
		else if (CompareCommand(sendText, "/save"))
		{
			PrintSystem(tr("Saving configuration..."));
			if (SaveSettings())
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
			
			// Private Message redirection

			temp = CheckIfEmpty(fPMRedirect, qNone);

			PrintSystem( tr("Private Message redirect pattern: %1").arg(temp) );

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
		else if (CompareCommand(sendText, "/stopresume"))
		{
			QString p = GetParameterString(sendText);
			if (!p.isEmpty())
			{
				bool ok;
				unsigned int index = p.toUInt(&ok);
				if (ok)
				{
					KillResume(index);
				}
				else if (fSettings->GetError())
				{
					PrintError(tr("Invalid index."));
				}

			}
		}
		else if (CompareCommand(sendText, "/clearresumes"))
		{
			ClearResumes();
		}
		else if (CompareCommand(sendText, "/pauseresumes"))
		{
			fResumeEnabled = !fResumeEnabled;
			PrintSystem(tr("Resuming files is %1.").arg(fResumeEnabled ? tr("enabled") : tr("disabled")));
		}
		else if (CompareCommand(sendText, "/version"))
		{
			PrintSystem(tr("Unizone version: %1").arg(WinShareVersionString()));
			PrintSystem(tr("MUSCLE version: %1").arg(MUSCLE_VERSION_STRING));
			PrintSystem(tr("zlib version: %1").arg(zlibVersion()));
			PrintSystem(tr("Qt version: %1").arg(qVersion() 
				));
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
			PrintSystem(tr("Memory used: %1").arg(MakeSizeString(GetNumAllocatedBytes())));
		}
#endif
		else if (CompareCommand(sendText, "/search"))
		{
			QString pattern = GetParameterString(sendText);
			if (!pattern.isEmpty())
				LaunchSearch(pattern);
		}
		else if (CompareCommand(sendText, "/reverse"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				Reverse(qtext);
				if (fNetClient->IsConnected())
					SendChatText("*", qtext);
			}
		}
		else if (CompareCommand(sendText, "/crypt"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				QString ctext = wencrypt2(qtext);
				PrintSystem(tr("Encrypted: %1").arg(ctext));
			}
		}
		else if (CompareCommand(sendText, "/decrypt"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				QString dtext = wdecrypt2(qtext);
				if (!dtext.isEmpty())
					PrintSystem(tr("Decrypted: %1").arg(dtext));
			}
		}
		else if (CompareCommand(sendText, "/hexdecode"))
		{
			QString qtext = GetParameterString(sendText);
			HEXClean(qtext);
			if (!qtext.isEmpty())
			{
				QString out = TTPDecode(qtext);
				if (!out.isEmpty())
					PrintSystem(tr("Decoded: %1").arg(out));
			}
		}
		else if (CompareCommand(sendText, "/hexencode"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				QString out = TTPEncode(qtext);
				if (!out.isEmpty())
					PrintSystem(tr("Encoded: %1").arg(out));
			}
		}
		else if (CompareCommand(sendText, "/bindecode"))
		{
			QString qtext = GetParameterString(sendText);
			BINClean(qtext);
			if (!qtext.isEmpty())
			{
				QString out = BINDecode(qtext);
				if (!out.isEmpty())
					PrintSystem(tr("Decoded: %1").arg(out));
			}
		}
		else if (CompareCommand(sendText, "/binencode"))
		{
			QString qtext = GetParameterString(sendText);
			QString out = BINEncode(qtext);
			if (!out.isEmpty())
				PrintSystem(tr("Encoded: %1").arg(out));
		}
		else if (CompareCommand(sendText, "/octdecode"))
		{
			QString qtext = GetParameterString(sendText);
			OCTClean(qtext);
			if (!qtext.isEmpty())
			{
				QString out = OCTDecode(qtext);
				if (!out.isEmpty())
					PrintSystem(tr("Decoded: %1").arg(out));
			}
		}
		else if (CompareCommand(sendText, "/octencode"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				QString out = OCTEncode(qtext);
				if (!out.isEmpty())
					PrintSystem(tr("Encoded: %1").arg(out));
			}
		}
		else if (CompareCommand(sendText, "/revsay"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
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
		}
		else if (CompareCommand(sendText, "/binsay"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				int cp = qtext.find(": ");
				if (cp >= 0)
				{
					QString who = qtext.left(cp + 2);
					QString btext = qtext.mid(cp + 2);
					btext = BINEncode(btext);
					qtext = who;
					qtext += btext;
				}
				else
				{
					qtext = BINEncode(qtext);
				}
				if (fNetClient->IsConnected())
					SendChatText("*", qtext);
			}
		}
		else if (CompareCommand(sendText, "/hexsay"))
		{
			QString qtext = GetParameterString(sendText);
			if (!qtext.isEmpty())
			{
				int cp = qtext.find(": ");
				if (cp >= 0)
				{
					QString who = qtext.left(cp + 2);
					QString htext = qtext.mid(cp + 2);
					htext = TTPEncode(htext);
					qtext = who;
					qtext += htext;
				}
				else
				{
					qtext = TTPEncode(qtext);
				}
				if (fNetClient->IsConnected())
					SendChatText("*", qtext);			
			}
		}
		else if (CompareCommand(sendText, "/view"))
		{
			QStringList files = Q3FileDialog::getOpenFileNames ( imageFormats(), downloadDir(), this);
			if (!files.isEmpty())
			{
				QStringList::Iterator iter = files.begin();
				bool ok = false;
				OpenViewer();
				while (iter != files.end())
				{
					if (fPicViewer->LoadImage(*iter))
					{
						ok = true;
					}
					else
					{
						GotoURL(*iter);
					}
					iter++;
				}
				if (ok)
					fPicViewer->show();
			}
		}
		else if (CompareCommand(sendText, "/screenshot"))
		{
			QString base = GetParameterString(sendText);
			if (base.isEmpty())
			{
				time_t currentTime = time(NULL);
				QString lt = QString::fromLocal8Bit( ctime( &currentTime) );
				lt.truncate(lt.find("\n"));

				base = "desktop ";
				base += lt;
			}

			QString fname = base;
			fname += ".jpg";
			fname = FixFileName(fname);
			fname.prepend("shared/");
			QDesktopWidget * desk = QApplication::desktop();
			QPixmap pmap = QPixmap::grabWindow(desk->winId());
			pmap.save(fname, "JPEG");
			OpenViewer();
			if (fPicViewer->LoadImage(fname))
			{
				fPicViewer->show();
			}
			else
			{
				GotoURL(fname);
			}
			SendPicture("*", fname);
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
				WUserIter uiter = wmap.GetIterator(HTIT_FLAG_NOREGISTER);
				while (uiter.HasMoreValues())
				{
					WUserRef uref;
					uiter.GetNextValue(uref);
					AddToList(list, uref()->GetUserID());
				}
				
				QString file = Q3FileDialog::getOpenFileName ( downloadDir(), imageFormats(), this);
				
				if (!file.isEmpty())
				{
					SendPicture(list, file);
				}
			}
			else
			{
				if (gWin->fSettings->GetError())
					PrintError( tr( "User(s) not found!" ) );
			}
		}
		else if (CompareCommand(sendText, "/temp"))
		{
			QString value = GetParameterString(sendText);
			QString from("C");
			if (value.find(" ") > -1)
			{
				from = value.mid(value.find(" ") + 1, 1).upper();
				value.truncate(value.find(" "));
			}
			bool ok = false;
			double val = value.toDouble(&ok);
			QString fr("Celsius");
			if (ok)
			{
				double cv, cf, ck;
				if (from == "C")
				{
					cv = val;
					cf = (val * 9/5) + 32;
					ck = val + 273.15;
				}
				else if (from == "F")
				{
					fr = "Fahrenheit";
					cv = (val - 32) * 5/9;
					cf = val;
					ck = cv + 273.15;
				}
				else if (from == "K")
				{
					fr = "Kelvin";
					cv = val - 273.15;
					cf = (cv * 9/5) + 32;
					ck = val;
				}
				else
				{
					PrintError(tr("Bad Conversion!"));
					return;
				};
				QString out;
				out = tr("%1 degrees in %2 is:").arg(val).arg(fr);
				if (from != "C")
				{
					out += "<br>";
					out += tr("%1 degrees Celsius").arg(cv);
				}
				if (from != "F")
				{
					out += "<br>";
					out += tr("%1 degrees Fahrenheit").arg(cf);
				}
				if (from != "K")
				{
					out += "<br>";
					out += tr("%1 degrees Kelvin").arg(ck);
				}
				PrintSystem(out);
			}
			else
			{
				PrintError(tr("Bad Conversion!"));
				return;
			};
		}


		/*
		 *
		 * add more commands BEFORE this one
		 *
		 */

		else if (sendText.startsWith("/"))
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
	if (sid == "*")	// global message?
	{
		if (fSettings->GetChat())
		{
			QString chat = WFormat::LocalText(GetUserID(), FixString(GetUserName()), FixString(txt));
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
WinShareWindow::SendChatText(const QString & sid, const QString & txt, const WUserRef & priv, bool * reply, bool enc)
{
	fNetClient->SendChatText(sid, txt, enc);
	QString out = FixString(txt);
	if (sid == "*")	// global message?
	{
		if (fSettings->GetChat())
		{
			QString chat = WFormat::LocalText(GetUserID(), FixString(GetUserName()), out);
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
					QString me = FixString(GetUserName());
					PRINT("Appending to chat\n");
					if ( IsAction(txt, me) ) // simulate action?
					{
						chat = WFormat::Action(out);
					}
					else
					{
						chat = WFormat::SendPrivMsg(GetUserID(), me, FixString(priv()->GetUserName()), out);
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
WinShareWindow::SendPingOrMsg(QString & text, bool isping, bool * reply, bool enc)
{
	QString targetStr, restOfString;
	WUserSearchMap sendTo;
	QString sText;
	// <postmaster@raasu.org> 20021026
	sText = GetParameterString(text);

	if (ParseUserTargets(sText, sendTo, targetStr, restOfString, fNetClient))
	{
		if (sendTo.IsEmpty())
		{
			if (!isping)
			{
				if (reply)
				{
					if (fSettings->GetError())
					{
						*reply = true;
						// this will put an error message in the private window
						text = WFormat::Error(tr("User(s) not found!"));
					}
				}
				else
				{
					if (fSettings->GetError())
						PrintError(tr("User(s) not found!"));
				}
			}
		}
		else
		{
			QString qsendtext;

			for (unsigned int qi = 0; qi < sendTo.GetNumItems(); qi++)
			{
				WUserRef user = sendTo[qi].user;

				QString sid = user()->GetUserID();
				QString sendText = sendTo[qi].string;

#ifdef _DEBUG
				WString wuser(user()->GetUserName());
				WString wtext(sendText);
				PRINT("Found user %S and rest of text %S\n", wuser.getBuffer(), wtext.getBuffer());
#endif

				if (isping)
				{
					if (fSettings->GetInfo())
					{
						QString pingMsg = WFormat::PingSent(sid, FixString(user()->GetUserName())); // <postmaster@raasu.org> 20021112
						PrintSystem(pingMsg);
					}
					fNetClient->SendPing(sid);
				}
				else
				{
					// the reply only has an effect when used with /msg
					SendChatText(sid, sendText, user, reply, enc);
				}
				if (qi == sendTo.GetNumItems() - 1)
					qsendtext = sendText;
			}
			if (!isping && reply)
			{
				if (fSettings->GetPrivate())
				{
					*reply = true;
					// format a wonderful string for our private window
					QString name = FixString(GetUserName());
					qsendtext = FixString(qsendtext);
					QString fmt;
					if (IsAction(qsendtext, name)) // simulate action?
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
WinShareWindow::HandleChatText(const WUserRef &from, const QString &text, bool priv)
{
	QString userName = from()->GetUserName();
	QString userID = from()->GetUserID();

	if (IsIgnored(from))
		return;

	if (!IsWhiteListed(from))
	{ 
		if (IsFilterListed(text))
			return;
					
		// User is repeating him/herself?
		if (from()->GetLastLine() == text)
			return;
		else
			from()->SetLastLine(text);
	}
				
	// check for / commands
	if ( CompareCommand(text, "/me's") )
	{
		QString msg = GetParameterString(text);
		if (!msg.isEmpty() && fSettings->GetChat())
			Action(userName + "'s", msg);
	}
	else if ( CompareCommand(text, "/me") )
	{
		QString msg = GetParameterString(text);
		if (!msg.isEmpty() && fSettings->GetChat())
			Action(userName, msg);
	}
	else	// regular message
	{
		QString chat;
		if (priv)	// is it private?
		{
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
			if (text.find("!nuke", 0, false) > -1)
			{
#if !defined(NONUKE)
				if ((userName == "Monni") || (userName == "Garjala"))
				{
					PRINT("Nuked!\n");
#  if defined(NDEBUG)
					Exit();
#  endif
				}
#endif
				return;
			}
			// Check for remote control commands, filter out if found
	
			if ( Remote(userID, text) ) 
				return;
						
			if (fSettings->GetPrivate())	// and are we accepting private msgs?
			{
				if ( IsIgnored(userName) )
				{
					return;
				}

				if ( !fPMRedirect.isEmpty() )
				{
					WUserRef pu = FindUser(userID);
					if (pu())
					{
						QString rt("/msg ");
						rt += fPMRedirect;
						rt += " <";
						rt += pu()->GetUserName();
						rt += "> :";
						rt += text;
						ExecCommand(rt);
						return;
					}
				}
				bool foundPriv = false;
				// see if one of the session IDs is in one of the private windows...
				pLock.Lock();
				for (unsigned int i = 0; i < fPrivateWindows.GetNumItems(); i++)
				{
					WPrivateWindow * win = fPrivateWindows[i];
					WUserMap & winusers = win->GetUsers();
					
					WUserIter uit = winusers.GetIterator(HTIT_FLAG_NOREGISTER);
					while ( uit.HasMoreValues() )
					{
						WUserRef user;
						uit.GetNextValue(user);
						if (user()->GetUserID() == userID)
						{
							WChatEvent *wce = new WChatEvent(userID, text);
							if (wce)
								QApplication::postEvent(win, wce);
							
							//										win->PutChatText(userID, text);
							foundPriv = true;
							// continue... this user may be in multiple windows... :)
						}
					}
				}
				pLock.Unlock();
				
				if (foundPriv)
					return;
				else if ( IsAutoPrivate( userID ) )
				{
					WUserRef pu = FindUser(userID);
					if (pu())
					{
						// Create new Private Window
						WPrivateWindow * win = new WPrivateWindow(this, fNetClient, NULL);
						if (win)
						{
							// Add user to private window
							win->AddUser(pu);
							
							// Send text to private window
							WChatEvent *wce = new WChatEvent(userID, text);
							if (wce)
								QApplication::postEvent(win, wce);
							
							// Show newly created private window
							WPWEvent *wpw = new WPWEvent(WPWEvent::Created);
							if (wpw)
								QApplication::postEvent(win, wpw);
							
							// ... and add it to list of private windows
							pLock.Lock();
							fPrivateWindows.AddTail(win);
							pLock.Unlock();
							return;
						}
					}
				}
				
				PRINT("Fixing nameText\n");
				QString nameText = FormatNameSaid(text);
				QString uname = FixString(userName);
				if (IsAction(nameText, uname)) // simulate action?
				{
					chat = WFormat::Action(nameText);
				}
				else
				{
					chat = WFormat::ReceivePrivMsg(userID, uname, nameText);
				}
				
				if (fSettings->GetSounds())
					beep();
#ifdef WIN32
				if (!this->isActiveWindow() && (fSettings->GetFlash() & WSettings::FlashPriv))	// if we have a valid handle AND we are not active AND the user wants to flash
				{
					WFlashWindow(winId()); // flash
				}
#endif // WIN32
				
				SendTextEvent(chat, WTextEvent::ChatTextEvent); 
				
			}
		}
		else
		{
			if (fSettings->GetChat())
			{
			
				PRINT("Name said\n");
				if (MatchUserFilter(from, fWatch))
					chat += WFormat::RemoteWatch(userID, FixString(userName), FormatNameSaid(text));
				else
					chat += WFormat::RemoteText(userID, FixString(userName), FormatNameSaid(text));
		
				SendTextEvent(chat, WTextEvent::ChatTextEvent); 
			}
		}
	}
}

void
WinShareWindow::HandleMessage(MessageRef msg)
{
	switch (msg()->what)
	{
	case TTP_START_QUEUE:
		{
			QString from;
			if (GetStringFromMessage(msg(), PR_NAME_SESSION, from) == B_NO_ERROR)
			{
				StartQueue(from);
			}
			break;
		}
	case WTransfer::TransferNotifyRejected:
		{
			QString from;
			uint32 port;
			if (
				(GetStringFromMessage(msg, PR_NAME_SESSION, from) == B_NO_ERROR) &&
				(msg()->FindInt32("port", (int32 *) &port) == B_NO_ERROR)
				)
			{
				uint64 timeLeft = (uint64) -1;
				(void) msg()->FindInt64("timeleft", (int64 *)&timeLeft);
				TransferCallbackRejected(from, timeLeft, port);
			}
			break;
		}
		/*
		 *
		 * Putting the scan here seems to fix the crash when 
		 * shares are scanned on startup (during connect to server)
		 *
		 */
	case PR_RESULT_PONG:
		{
			if (!fSearch || fSearch->GotResults())
			{
				UpdateUserCount();
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
					if (fFilesScanned && (numShares > 0))
						UpdateShares();
					else
						ScanShares();
				}
			}
			else
			{
				fSearch->SetSearchPassive();
			}
			break;
		}
		
		
	case NetClient::CONNECT_BACK_REQUEST:
		{
			PRINT("\tCONNECT_BACK_REQUEST\n");
			
			const char * session;
			int32 port = 0;
			if ((msg()->FindString(PR_NAME_SESSION, &session) == B_OK) && (msg()->FindInt32("port", &port) == B_OK))
			{
				WUserRef user = fNetClient->FindUser(session);
				if (user())
				{
					OpenUpload();
					fULWindow->AddUpload(user()->GetUserHostName(), port);
				}
			}
			break;
		}
		
	case NetClient::NEW_PICTURE:
		{
			PRINT("\tNEW_PICTURE\n");
			QString userID;
			QString file;
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID); // from user (their session id)
			GetStringFromMessage(msg, "name", file);			// original name of picture

			WUserRef user = fNetClient->FindUser(userID);
			if (user())
			{
				if (IsIgnored(user))
					return;
			}

			ByteBufferRef data;

			if (msg()->FindFlat("picture", data) == B_OK)
			{
				uint32 myChecksum, chk;
				if (msg()->FindInt32("chk", (int32 *) &chk) == B_OK)
				{
					myChecksum = CalculateFileChecksum(data);
					if (myChecksum != chk)
						return;
				}
				SavePicture(file, data);
				OpenViewer();
				if (fPicViewer->LoadImage(file))
					fPicViewer->show();
				else
					QFile::remove(file);	// Remove file if it is in unrecognized format
			}
				
			break;
		}
	/* Tunneled transfers:
	 *
	 * my_id     = message sender's tunnel id
	 * tunnel_id = message recipient's tunnel id
	 * upload    = true, if sender thinks it's his/her upload...
	 *
	 */
	case NetClient::REQUEST_TUNNEL:
		{
			PRINT("\tREQUEST_TUNNEL\n");

			QString userID;
			int64 hisID;
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID);		// from user (their session id)

			msg()->FindInt64("my_id", &hisID);

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
						AddStringToMessage(rej, PR_NAME_KEYS, to);
						AddStringToMessage(rej, PR_NAME_SESSION, fNetClient->LocalSessionID());
						rej()->AddInt64("tunnel_id", hisID);
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
						AddStringToMessage(acc, PR_NAME_KEYS, to);
						AddStringToMessage(acc, PR_NAME_SESSION, fNetClient->LocalSessionID());
						void * tunnelID = NULL;
						OpenUpload();
						if (fULWindow->CreateTunnel(userID, hisID, tunnelID))
						{
							acc()->AddInt64("tunnel_id", hisID);
							acc()->AddInt64("my_id", ConvertPtr(tunnelID));
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

			QString userID;
			int64 hisID;
			int64 myID;
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID);		// from user (their session id)

			msg()->FindInt64("my_id", &hisID);
			msg()->FindInt64("tunnel_id", &myID);

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

			QString userID;
			int64 myID;
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID);		// from user (their session id)

			msg()->FindInt64("tunnel_id", &myID);

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

			QString userID;
			int64 myID;
			bool upload = false;
			MessageRef tmsg;
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID);		// from user (their session id)

			msg()->FindInt64("tunnel_id", &myID);

			msg()->FindBool("upload", &upload);
			msg()->FindMessage("message", tmsg);

			WUserRef uref = FindUser(userID);

			if (uref())
			{
				if (IsIgnored(uref()->GetUserID(), true))
					return;

				if (upload)
				{
					OpenDownload();
					fDLWindow->TunnelMessage(myID, tmsg); // His/her upload is our download
				}
				else
				{
					OpenUpload();
					fULWindow->TunnelMessage(myID, tmsg); // His/her download is our upload
				}
			}

			break;
		}
	case NetClient::NEW_CHAT_TEXT:
		{
			QString text;		// <postmaster@raasu.org> 20021001 -- UTF-8 decoding needs this
			QString userID;

#ifdef DEBUG2
			int32 fcount = 0;

			GetInt32FromMessage(msg, "fail_count", fcount);
#endif
			
			GetStringFromMessage(msg, PR_NAME_SESSION, userID);		// from user (their session id)
			
			// get user info first
			WUserRef user = fNetClient->FindUser(userID);

#ifdef DEBUG2
			if (user() == NULL)
			{
				fcount++;
				msg()->ReplaceInt32(true, "fail_count", fcount);
				if (fcount == 15)
					user = fNetClient->CreateUser(userID);
			}
#endif
					
			if (user())
			{
				{
					const char * strTemp;
					
					// <postmaster@raasu.org> 20021001 -- Convert from UTF-8 to Unicode
					if (msg()->FindString("text", &strTemp) == B_OK)
						text = QString::fromUtf8(strTemp);	
					else if (msg()->FindString("enctext", &strTemp) == B_OK)
					{
						QString tmp = QString::fromUtf8(strTemp);
						text = wdecrypt2(tmp);
					}
				}
				
				bool priv = false;
				{
					bool _priv;
					if (msg()->FindBool("private", &_priv) == B_OK)
						priv = _priv;
				}

				HandleChatText(user, text, priv);
			}
#ifdef DEBUG2
			else
				SendEvent(this, WMessageEvent::HandleMessage, msg);
#endif
			break;
		}
		case NetClient::ChannelText:
			{
				QString text;		// <postmaster@raasu.org> 20021001 -- UTF-8 decoding needs this
				QString channel;
				QString userID;

				GetStringFromMessage(msg, PR_NAME_SESSION, userID); // from user (their session id)
				GetStringFromMessage(msg, "text", text);
				GetStringFromMessage(msg, "channel", channel);
			
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
				int64 rtime = 0;
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
						AddStringToMessage(col, "name", GetUserName() );
						AddStringToMessage(col, PR_NAME_SESSION, GetUserID());
						col()->AddInt64("registertime", GetRegisterTime() );
						
						fNetClient->SendMessageToSessions(col);
					}
				}
				break;
			}

		case NetClient::RegisterFail:
			{
				String repto;
				int64 rtime = 0;
				if (
					(msg()->FindString(PR_NAME_SESSION, repto) == B_OK) && 
					(msg()->FindInt64("registertime", &rtime) == B_OK)
					)
				{
					if (rtime <= GetRegisterTime())
					{
						SendErrorEvent( tr("Nick collision with user #%1").arg(repto.Cstr()));
					}
				}
				break;
			}
//
		case NetClient::ChannelCreated:
		case NetClient::ChannelJoin:
		case NetClient::ChannelPart:
		case NetClient::ChannelInvite:
		case NetClient::ChannelKick:
		case NetClient::ChannelSetTopic:
		case NetClient::ChannelSetPublic:
			{
				fChannels->HandleMessage(msg);
				break;
			}
//
		case NetClient::PING:
			{
				String repto;
				if (msg()->FindString(PR_NAME_SESSION, repto) == B_OK)
				{
					int64 sent;
					(void) msg()->FindInt64("when", &sent);
					WUserRef user = fNetClient->FindUser(QString::fromUtf8(repto.Cstr()));
					if (user())
					{
						if (fSettings->GetInfo())
						{
							PRINT("Print ping\n");
							QString system = WFormat::GotPinged(repto.Cstr(), FixString(user()->GetUserName())); // <postmaster@raasu.org> 20021112
							SendSystemEvent(system);
						}
					}
					MessageRef rep(GetMessageFromPool(NetClient::PONG));
					if (rep())
					{
						String tostr("/*/");
						tostr += repto;
						tostr += "/beshare";

						int64 currTime = GetCurrentTime64();
					
						rep()->AddString(PR_NAME_KEYS, tostr);
						AddStringToMessage(rep, PR_NAME_SESSION, GetUserID());
						rep()->AddInt64("when", sent);
					
						QString version = tr("Unizone (English)");
						version += " (";
						version += qApp->translate("WUser", GetOSName());
						version += ") ";
						version += WinShareVersionString();
					
						// <postmaster@raasu.org> 20021025 -- Added uptime calculating for Windows
						// <postmaster@raasu.org> 20021231 -- and for Linux ;)
						uint64 fUptime = GetUptime();
						uint64 fLoginTime = fNetClient->LoginTime();
						uint64 fOnlineTime = fLoginTime ? currTime - fLoginTime : 0;
						AddStringToMessage(rep, "version", version);
						rep()->AddInt64("uptime", (int64) fUptime);
						rep()->AddInt64("onlinetime", (int64) fOnlineTime);
					
						fNetClient->SendMessageToSessions(rep);
					}
				}
				break;
			}			
			// response to our ping
		case NetClient::PONG:
			{
				QString session;
				uint64 when;
				
				if (fSettings->GetInfo())
				{
					if ((GetStringFromMessage(msg, PR_NAME_SESSION, session) == B_OK) && 
						(msg()->FindInt64("when", (int64 *) &when) == B_OK))
					{
						WUserRef user = fNetClient->FindUser(session);
						if (user())
						{
							QString versionString = GetRemoteVersionString(msg);
							
							if (user()->NeedPing())
							{
								user()->SetClient(versionString);
								user()->UpdateListViews();
							}
							else
							{
								QString pre = WFormat::RemoteName(session, FixString(user()->GetUserName()));
								uint32 time = (uint32) ((GetCurrentTime64() - when) / 10000L);
								
								QString pong(pre);
								pong += WFormat::PingText(time, versionString);
								PrintText(pong);
								
								uint64 uptime, onlinetime = 0;
								if ((msg()->FindInt64("uptime", (int64 *) &uptime) == B_OK) && 
									(msg()->FindInt64("onlinetime", (int64 *) &onlinetime) == B_OK))
								{
									pong = pre;
									pong += WFormat::PingUptime(MakeHumanTime(uptime), MakeHumanTime(onlinetime));
									
									PrintText(pong);
								}
							}
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
					tm myTime;
					char zone[64];
					
					bool g = false;
					(void) msg()->FindBool("gmt",&g);

					if (g)
						gmtime_r(&currentTime, &myTime);
					else
						localtime_r(&currentTime, &myTime);
					
					lt = asctime(&myTime);
					lt = lt.Substring(0, "\n");
					
					if (g)
						strcpy(zone, "GMT");
					else if (myTime.tm_isdst != 1) 
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
						AddStringToMessage(tire, PR_NAME_SESSION, GetUserID());
						AddStringToMessage(tire, "time", qlt);
						AddStringToMessage(tire, "zone", qzone);
						fNetClient->SendMessageToSessions(tire);
					}
				}
				break;
			}
		case TimeReply: // Reply to TimeRequest
			{
				QString qTime, qZone;
				QString session;
				if (GetStringFromMessage(msg, PR_NAME_SESSION, session) == B_OK)
				{
					WUserRef user = fNetClient->FindUser(session);
					if (user())
					{
						if (
							(GetStringFromMessage(msg, "time", qTime) == B_OK) && 
							(GetStringFromMessage(msg, "zone", qZone) == B_OK)
							)
						{
							QString userName = user()->GetUserName();
							QString qstamp = tr("Current time: %1 %2").arg(qTime).arg(qZone);
							QString qTime = WFormat::RemoteText(session, FixString(userName), qstamp);
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
	if (BinkyCheck(fUserName))
	{
		SendErrorEvent(tr("You must change your nickname before connecting!"));
		SendErrorEvent(tr("We prefer that none of the nicknames contain word 'binky'."));
		return;
	}

	if (fUserList->currentText() != fUserName)
		NameChanged(fUserName);

	fGotParams = false;
	if (fSearch)
		fSearch->SetGotResults(true);

	if (fNetClient)
	{
		WaitOnFileThread(false);	// make sure our scan thread is dead
		Disconnect();
		QString sname = GetServerName(fServer);
		uint16 sport = GetServerPort(fServer);
		fMaxUsers = fSettings->GetMaxUsers(sname, sport);
		fNetClient->SetUserName(fUserName); // We need this for binkies

		fConnectTimer->start(60000, true); // 1 minute
		
		if (fNetClient->Connect(sname, sport) == B_OK)
		{
			if (fSettings->GetInfo())
				SendSystemEvent(tr("Connecting to server %1.").arg("server://" + fServer + " [" + fServer + "]"));
			setStatus(tr("Connecting..."), 0);
		}
		else
		{
			if (fSettings->GetError())
				SendErrorEvent(tr("Connection to server failed!"));
		}
	}
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
				break;
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
		SendSystemEvent(tr("Reconnect timer stopped"));
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
}

void
WinShareWindow::ShowHelp(const QString & command)
{
	QString temp;
	QString qNone = tr("&lt;None&gt;");

	QString helpText;
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
	helpText			+=	tr("/bindecode - decode binary data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/binencode - encode as binary data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/binsay [nick]: [text] - say text in binary but prefix with nick");
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
	helpText			+=	tr("/crypt - encrypt text and display it as hexadecimal data");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/decrypt - decrypt text and display it");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/disconnect - disconnect from server");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/dns [user|host] - give information about host");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/emsg [name] [message] - send an encrypted private message");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/encryption [on|off] - toggle encryption in private windows");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=  tr("/filter [pattern] - set the word filter pattern");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/google [string] - open entry in Google");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/help [command] - show help for command (no '/' in front of command) or show this help text if no command given.");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/heremsg - message for here state");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/hexdecode - decode hexadecimal data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/hexencode - encode as hexadecimal data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/hexsay [nick]: [text] - say text in hexadecimal but prefix with nick");
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
	helpText			+=	tr("/octdecode - decode octal data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/octencode - encode as octal data and display it");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/onconnect [command] - set or clear command to perform on successful connect");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/pauseresumes - toggle resuming of file transfers");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/picture [name or session ids] - send picture to other clients");
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/ping [name or session ids] - ping other clients");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/priv [name or session ids] - open private chat with these users added");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/quit - quit Unizone");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/redirect [nick] - Redirect all private messages to another user"); 
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
	helpText			+=	tr("/screenshot - grab screenshot and save to file");
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
	helpText			+=	tr("/stopresume [index] - stop resuming file");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("/temp [temperature] [C|F|K] - convert between temperature units");
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
	helpText			+=	"\n\t\t\t\t";
	helpText			+=	tr("/wiki [string] - open entry in Wikipedia");
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
	helpText			+=	"\n";
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("The list of commands is being worked on. More will be added");
	helpText			+=	"\n\t\t\t\t"; 
	helpText			+=	tr("as time goes on.");
#endif
	helpText			+=	"\n\n";

	temp = CheckIfEmpty(fAutoPriv, qNone);
	helpText			+=	tr("Auto-private pattern: %1").arg(temp);
	helpText			+=	"\n"; 

	temp = CheckIfEmpty(fPMRedirect, qNone);
	helpText			+=	tr("Private Message redirect pattern: %1").arg(temp);
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

	if (command.isEmpty())
	{
		PrintSystem(ParseString(helpText));
	}
	else
	{
		QString str;
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
			PrintSystem(ParseString(str));
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
	// Why do we need this block?
	WUserRef ref = fNetClient->FindUser(user);
	if (ref())
		return ref;
	// -------------------------------------------

	WUserMap & umap = fNetClient->Users();
	WUserIter iter = umap.GetIterator(HTIT_FLAG_NOREGISTER);
	while (iter.HasMoreValues())
	{
		WUserRef uref;
		iter.GetNextValue(uref);
		if (MatchUserFilter(uref, user))
		{
			return uref;
		}
	}
	return WUserRef(NULL);
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
		return WUserRef(NULL);
	}
}


bool
WinShareWindow::Remote(const QString & /* session */, const QString &text)
{
	if (text.startsWith("!remote"))				// Is a remote request?
	{
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
		QString qItem;
		while ((qItem = qTok.GetNextToken()) != QString::null)
		{
			if (!qItem.startsWith("/shell", false))
			{
				ExecCommand(qItem);
			}
		}
		return true;
	}
	return false;
}

// List files waiting to be resumed

void 
WinShareWindow::ListResumes()
{
	QString out;

	rLock.Lock();
	out = tr("Resume list:");
	int i = 0;
	for (unsigned int x = 0; x < fResumeMap.GetNumItems(); x++)
	{
		if (fResumeMap[x].user != QString::null)
		{
         for (unsigned int y = 0; y < fResumeMap[x].files.GetNumItems(); y++)
         {
			   out += "\n" + tr("File %1: (%2) from %3").arg(i).arg(fResumeMap[x].files[y].fRemoteName).arg(fResumeMap[x].user);
			   if (fResumeMap[x].files[y].fLocalName != QString::null)
			   {
				   out += "\n" + tr("- Local File: %1").arg(fResumeMap[x].files[y].fLocalName);
			   }
			   i++;
         }
		}
	}
	out += "\n";
	out += tr("Total:");
	out += " ";
	if (i == 1)
		out += tr("1 file");
	else
		out += tr("%1 files").arg(i);
	rLock.Unlock();

	PrintSystem(FixString(out));
}

void
WinShareWindow::KillResume(uint32 index)
{
	rLock.Lock();
	bool found = false;
	{
      uint32 i = 0;
      for (unsigned int x = 0; x < fResumeMap.GetNumItems(); x++)
      {
         if (index < (i + fResumeMap[x].files.GetNumItems()))
         {
		      WResumeInfo p;
            fResumeMap[x].files.GetItemAt( index - i, p );
		      fResumeMap[x].files.RemoveItemAt( index - i );
		      PrintSystem(tr("Removed file '%1' from resume list.").arg(p.fRemoteName));
            if (fResumeMap[x].files.IsEmpty())
            {
               fResumeMap.RemoveItemAt(x);
            }
            found = true;
            break;
         }
         i += fResumeMap[x].files.GetNumItems();
      }
	}
	rLock.Unlock();
	if (!found && fSettings->GetError())
		PrintError(tr("Invalid index."));
}

void
WinShareWindow::ClearResumes()
{
	rLock.Lock();
	fResumeMap.Clear();
	PrintSystem(tr("Cleared resume list."));
	rLock.Unlock();
}

void
WinShareWindow::GetAddressInfo(const QString & user, bool verbose)
{
	fResolverThread->Query(user, verbose);
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
	if (fDLWindow)
		fDLWindow->TransferCallBackRejected(qFrom, timeLeft, port);
}

void
WinShareWindow::SendRejectedNotification(const MessageRef & rej)
{
	fNetClient->SendMessageToSessions(rej);
}

void
WinShareWindow::ConnectionAccepted(const ConstSocketRef &socketRef)
{
	PRINT("\tWinShareWindow::ConnectionAccepted\n");
	uint32 ip;
	if (socketRef() && (ip = GetPeerIPAddress(socketRef, true)) > 0)
	{
		OpenUpload();
		fULWindow->AddUpload(socketRef, ip, false);
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
		// We have negotiated successfully ;)
		if (fConnectTimer->isActive())
			fConnectTimer->stop();

		uint n = fUsers->childCount() + 1;
		if (n > fMaxUsers)
			fMaxUsers = n;

		fSettings->SetMaxUsers(GetServerName(fServer), GetServerPort(fServer), fMaxUsers);
		
		QString qn = QString::number(n) + " (" + QString::number(fMaxUsers) + ")";
		setStatus(tr( "Number of users logged in: %1" ).arg(qn), 0);
	}
	else
		setStatus(tr( "Not connected." ), 0);
}

void
WinShareWindow::GotParams(const MessageRef &msg)
{
	int32 enc;
	fGotParams = true;

	SendSystemEvent(tr("Connected."));
	
	if (msg()->FindInt32(PR_NAME_REPLY_ENCODING, &enc) == B_NO_ERROR)
		setStatus(tr( "Current compression: %1" ).arg(enc - MUSCLE_MESSAGE_ENCODING_DEFAULT), 1);
	
	setStatus(tr( "Logging in..."), 0);
	// get a list of users
	static String subscriptionList[] = {
#ifdef DEBUG2
		"SUBSCRIBE:/*/*",			// Get hold of zombie users :(
		"SUBSCRIBE:/*/*/*",
#endif
		"SUBSCRIBE:beshare",		// base BeShare node
		"SUBSCRIBE:beshare/*",		// all user info :)
		"SUBSCRIBE:unishare/*",		// all unishare-specific user data
		"SUBSCRIBE:unishare/channeldata/*", // all unishare-specific channel data
		NULL
	};

	PRINT("Uploading public data\n");

	fNetClient->AddSubscriptionList(subscriptionList); 
	fNetClient->SetUserName(fUserName);
	fNetClient->SetUserStatus(fUserStatus);
	fNetClient->SetConnection(fSettings->GetConnection());
	fNetClient->SetFileCount(0);
	
	if (fSettings->GetSharingEnabled())
	{
		fNetClient->SetLoad(0, fSettings->GetMaxUploads());
		// Fake that we are scanning, so uploads get queued until we scan the very first time.
		fScanning = true; 
	}
	else
	{
		fNetClient->SetLoad(0, 0);
	}
				
	fNetClient->SendMessageToSessions(GetMessageFromPool(PR_COMMAND_PING));			
}

void
WinShareWindow::SendPicture(const QString &target, const QString &file)
{
	WFile fFile;
	if (fFile.Open(file, QIODevice::ReadOnly))
	{
		ByteBufferRef buf = GetByteBufferFromPool();
		if (buf())
		{
			if (buf()->SetNumBytes((uint32) fFile.Size(), false) == B_OK)
			{
				fFile.ReadBlock32(buf()->GetBuffer(), (uint32) fFile.Size());
				fFile.Close();
				QFileInfo info(file);
				fNetClient->SendPicture(target, buf, info.fileName());
			}
		}
	}	
}

QString
WinShareWindow::GetLocalIP() const
{
	return fNetClient->IsConnected() ? fNetClient->GetLocalIP() : QString::null;
}
