// English Strings

#ifndef LANG_ENGLISH_H
#define LANG_ENGLISH_H

//
// Search Window
//

#define MSG_SW_CAPTION			"Search"

#define MSG_SW_CSEARCH			"Search:"
#define MSG_SW_FILENAME			"File Name"
#define MSG_SW_FILESIZE			"File Size"
#define MSG_SW_FILETYPE			"File Type"
#define MSG_SW_MODIFIED			"Modified"
#define MSG_SW_PATH				"Path"
#define MSG_SW_USER				"User"
#define MSG_SW_DOWNLOAD			"Download"
#define MSG_SW_CLOSE			"Close"
#define MSG_SW_CLEAR			"Clear"
#define MSG_SW_STOP				"Stop"

#define MSG_IDLE				"Idle."
#define MSG_WF_RESULTS			"Results: %1"
#define MSG_SEARCHING			"Searching for: \"%1\"."
#define MSG_WAIT_FOR_FST		"Waiting for file scan thread to finish..."

// Main Window

#define NAME				"Unizone (English)"				// Application Title

#define MSG_CSTATUS			"Status:"
#define MSG_CSERVER			"Server:"
#define MSG_CNICK			"Nick:"

#define MSG_CONNECTING      "Connecting to server %1."
#define MSG_CONNECTFAIL		"Connection to server failed!"
#define MSG_CONNECTED       "Connected."
#define MSG_DISCONNECTED	"Disconnected from server."
#define MSG_NOTCONNECTED    "Not connected."
#define MSG_RESCAN          "Rescanning shared files..."
#define MSG_SCAN_ERR1		"Already scanning!"
#define MSG_NOTSHARING		"File sharing not enabled."
#define MSG_NONICK			"No nickname passed."
#define MSG_NOMESSAGE		"No message to send."
#define MSG_AWAYMESSAGE		"Away message set to %1."
#define MSG_HEREMESSAGE		"Here message set to %1."
#define MSG_UNKNOWNCMD		"Unknown command!"
#define MSG_NOUSERS			"No users passed."
#define MSG_NO_INDEX		"No index specified."
#define MSG_INVALID_INDEX	"Invalid index."
#define MSG_USERSNOTFOUND   "User(s) not found!"
#define MSG_PINGSENT		"Ping sent to user #%1 (a.k.a. <font color=\"%3\">%2</font>)."
#define MSG_NEWVERSION		" %1 is available at http://www.raasu.org/tools/windows/."
#define MSG_SCANSHARES		"Scanning shares..."
#define MSG_SHARECOUNT		"Sharing %1 file(s)."
#define MSG_NAMECHANGED		"Name changed to <font color=\"%2\">%1</font>."
#define MSG_UNKNOWN			"Unknown"
#define MSG_USERHOST		"<font color=\"%3\">%1</font>'s IP address is %2."
#define MSG_NUM_USERS		"Number of users logged in: %1\n"

#define MSG_UPTIME			"Uptime: %1"
#define MSG_LOGGED			"Logged In: %1"
#define MSG_WEEK			"week"
#define MSG_WEEKS			"weeks"
#define MSG_DAY				"day"
#define MSG_DAYS			"days"
#define MSG_HOUR			"hour"
#define MSG_HOURS			"hours"
#define MSG_MINUTE			"minute"
#define MSG_MINUTES			"minutes"
#define MSG_SECOND			"second"
#define MSG_SECONDS			"seconds"
#define MSG_AND				" and "

#define MSG_HERE			"here"
#define MSG_AWAY			"away"
#define MSG_STATUS_IDLE		"idle"
#define MSG_STATUS_BUSY		"busy"
#define MSG_AT_WORK			"at work"
#define MSG_AROUND			"around"
#define MSG_SLEEPING		"sleeping"
//
// Menus
//

// Menu bar

#define MSG_FILE			"&File"
//#define MSG_AFILE			CTRL+Key_F
#define MSG_EDIT			"&Edit"
//#define MSG_AEDIT			CTRL+Key_E
//#define MSG_OPTIONS			"&Options"
//#define MSG_AOPTIONS		CTRL+Key_O
#define MSG_HELP			"&Help"
//#define MSG_AHELP			CTRL+Key_H

// File menu

#define MSG_CONNECT			"&Connect"
#define MSG_ACONNECT		CTRL+SHIFT+Key_C
#define MSG_DISCONNECT		"&Disconnect"
#define MSG_ADISCONNECT		CTRL+SHIFT+Key_D
#define MSG_OPEN_SHARED		"Open &Shared Folder"
#define MSG_AOPEN_SHARED	CTRL+Key_S
#define MSG_OPEN_DOWNLOAD	"Open &Downloads Folder"
#define MSG_AOPEN_DOWNLOAD	CTRL+Key_D
#define MSG_OPEN_LOGFOLDER	"Open &Logs Folder"
#define MSG_AOPEN_LOGFOLDER CTRL+Key_L
#define MSG_CLEAR_CHATLOG	"Cl&ear Chat Log"
#define MSG_ACLEAR_CHATLOG	CTRL+Key_E
#define MSG_SEARCH			"Search"
#define MSG_ASEARCH			CTRL+ALT+Key_S
#define MSG_EXIT			"E&xit"
#define MSG_AEXIT			ALT+Key_X

// Edit menu

#define MSG_PREFERENCES		"&Preferences"
#define MSG_APREFERENCES	CTRL+Key_P

// Help menu

#define MSG_ABOUT			"&About "
#define MSG_AABOUT			Key_F12

// Preferences window
//

#define MSG_PR_PREFERENCES		"Preferences"

// Tab names

#define MSG_GENERAL				"General"
#define MSG_CONNECTION			"Connection"
#define MSG_DISPLAY				"Display"
#define MSG_COLORS				"Colors"
#define MSG_STYLE				"Style"
#define MSG_FILE_SHARING		"File Sharing"
#define MSG_URL_LAUNCHING		"URL Launching"
#define MSG_CHTTP_LAUNCHER		"HTTP launcher:"
#define MSG_CFTP_LAUNCHER		"FTP launcher:"
#define MSG_CMAILTO_LAUNCHER	"Mailto: launcher:"
#define MSG_THROTTLING			"Throttling"

// General

#define MSG_CAUTOAWAY		"Auto Away:"
#define MSG_DISABLED		"Disabled"
#define MSG_2_MINUTES		"2 Minutes"
#define MSG_5_MINUTES		"5 Minutes"
#define MSG_10_MINUTES		"10 Minutes"
#define MSG_15_MINUTES		"15 Minutes"
#define MSG_20_MINUTES		"20 Minutes"
#define MSG_30_MINUTES		"30 Minutes"
#define MSG_1_HOUR			"1 Hour"
#define MSG_2_HOURS			"2 Hours"
#define MSG_AUTOUPDATE		"Auto Update Server List"
#define MSG_CHECK_NEW		"Check For New Versions"
#define MSG_LOGIN_ON_START	"Login On Startup"
#define MSG_ENABLE_LOGGING	"Enable Logging"
#define MSG_MULTI_COLOR_LISTVIEWS "Multi-color ListViews"

#define MSG_OK				"OK"
#define MSG_CANCEL			"Cancel"
#define MSG_TOGGLE_BLOCK	"(Un)block"

// Connection

#define MSG_CUPLOAD_BAND	"Upload Bandwidth:"
#define MSG_FIREWALLED		"I'm Firewalled"

// Display 

#define MSG_TIMESTAMPS		"Time Stamps"
#define MSG_USEREVENTS		"User Events"
#define MSG_UPLOADS			"Uploads"
#define MSG_CHAT			"Chat"
#define MSG_PRIVATE_MSGS	"Private Messages"
#define MSG_INFO_MSGS		"Info Messages"
#define MSG_WARNING_MSGS	"Warning Messages"
#define MSG_ERROR_MSGS		"Error Messages"
#define MSG_FLASH_WINDOW	"Flash Window When Mentioned"
#define MSG_FLASH_PRIVATE	"Flash Private Windows"
#define MSG_FONT_SIZE		"Font Size"

// Colors

#define MSG_CDESCRIPTION	"Description:"
#define MSG_CPREVIEW		"Preview:"
#define MSG_CHANGE			"Change"
#define MSG_LOCAL_NAME		"Local Name"
#define MSG_HLOCAL_NAME		"This is the color of your user name."
#define MSG_REMOTE_NAME		"Remote Name"
#define MSG_HREMOTE_NAME	"This is the color of other users' names."
#define MSG_REGULAR_TEXT	"Regular Text"
#define MSG_HREGULAR_TEXT	"This is the color of text sent by you and other users."
#define MSG_SYSTEM_TEXT		"System Text"
#define MSG_HSYSTEM_TEXT	"This is the color of \"System\"."
#define MSG_PING_TEXT		"Ping Text"
#define MSG_HPING_TEXT		"This is the color of the text in a ping response."
#define MSG_ERROR_TEXT		"Error Text"
#define MSG_HERROR_TEXT		"This is the color of \"Error\"."
#define MSG_ERRORMSG_TEXT	"Error Message Text"
#define MSG_HERRORMSG_TEXT	"This is the color of the text in error messages."
#define MSG_PRIVATE_TEXT	"Private Text"
#define MSG_HPRIVATE_TEXT	"This is the color of private text."
#define MSG_ACTION_TEXT		"Action Text"
#define MSG_HACTION_TEXT	"This is the color of \"Action\"."
#define MSG_URL_TEXT		"URL Text"
#define MSG_HURL_TEXT		"This is the color of URLs."
#define MSG_NAME_SAID_TEXT	"'Name Said' Text"
#define MSG_HNAME_SAID_TEXT "This is the color of your user name in text when someone says your name in the main chat."
#define MSG_HSTYLE			"The selected style will be applied as soon as it is selected."

// File Sharing

#define MSG_FS_ENABLED		"File sharing Enabled?"
#define MSG_BINKYNUKE		"Block binkies?"
#define MSG_BLOCK_DISCONNECTED "Block disconnected users?"
#define MSG_CFS_MAXUP		"Maximum Simultaneous Uploads:"
#define MSG_CFS_MAXDOWN		"Maximum Simultaneous Downloads:"

// Throttling

#define MSG_CCHAT				"Chat:"
#define MSG_TH_UPLOADS			"Uploads (per upload):"
#define MSG_TH_DOWNLOADS		"Downloads (per download):"
#define MSG_TH_BLOCKED			"Uploads (per blocked):"
#define MSG_UNLIMITED			"Unlimited"
#define MSG_NO_LIMIT			"No Limit"
#define MSG_BYTES				"bytes"

//
// Nick List
//

#define MSG_NL_NAME				"Name"
// ID is not translated Yet
#define MSG_NL_STATUS			"Status"
#define MSG_NL_FILES			"Files"
#define MSG_NL_CONNECTION		"Connection"
#define MSG_NL_LOAD				"Load"
#define MSG_NL_CLIENT			"Client"

#define MSG_NL_PRIVATECHAT		"Private Chat With %1"
#define MSG_NL_LISTALLFILES		"List All Files"
#define MSG_NL_GETIPADDR		"Get IP Address"
#define MSG_NL_GETADDRINFO		"Get Address Info"
#define MSG_REMOVE				"Remove"

//
// Format Strings
//

#define MSG_WF_USERCONNECTED		"User #%1 is now connected."
#define MSG_WF_USERDISCONNECTED		"User #%1 (a.k.a. <font color=\"%3\">%2</font>) has disconnected."
#define MSG_WF_USERNAMECHANGENO		"User #%1 is now known as <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED		"User #%1 (a.k.a. <font color=\"%4\">%2</font>) is now known as <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE		"User #%1 (a.k.a. <font color=\"%4\">%2</font>) is now %3."
#define MSG_WF_USERSTATUSCHANGE2	"User #%1 is now %2."
#define MSG_WF_STATUSCHANGED		"You are now %1."

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>System:</b> </font>"
#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">Ping returned in %3 milliseconds (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (Uptime: %3, Logged on for %4)</font>"
#define MSG_WF_ERROR			"<font color=\"%1\" size=\"%2\"><b>Error:</b></font> "
#define MSG_WF_WARNING			"<font color=\"%1\" size=\"%2\"><b>Warning:</b></font> "
#define MSG_WF_ACTION           "<font color=\"%1\" size=\"%2\"><b>Action:</b></font> "
#define MSG_WF_GOTPINGED		"<font color=\"%1\" size=\"%2\">User #%3 (a.k.a. <font color=\"%5\">%4</font>) pinged you.</font>"



//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define MSG_TX_STATUS			MSG_NL_STATUS
#endif

#define	MSG_TX_FILENAME			"Filename"
#define MSG_TX_RECEIVED			"Received"
#define	MSG_TX_TOTAL			"Total"
#define	MSG_TX_RATE				"Rate"
#define MSG_TX_ETA				"ETA"
#define MSG_TX_USER				"User"
#define MSG_TX_SENT				"Sent"
#define MSG_TX_QUEUE			"Queue"
#define MSG_TX_THROTTLE			"Throttle"
#define MSG_TX_MOVEUP			"Move Up"
#define MSG_TX_MOVEDOWN			"Move Down"
#define MSG_TX_CAPTION			"File Transfers"

#define MSG_TX_ISDOWNLOADING	"%1 is downloading %2."
#define MSG_TX_HASFINISHED		"%1 has finished downloading %2."
#define MSG_TX_FINISHED			"Finished downloading %2 from %1."

#endif