// German strings

#ifndef LANG_GERMAN_H
#define LANG_GERMAN_H

//
// Search Window
//

#define MSG_SW_CSEARCH				"Suche:"
#define MSG_SW_FILENAME				"Datei Name"
#define MSG_SW_FILESIZE				"Datei Größe"
#define MSG_SW_FILETYPE				"Datei Typ"
#define MSG_SW_MODIFIED				"Geändert"
#define MSG_SW_PATH					"Pfad"
#define MSG_SW_USER					"Benutzer"
#define MSG_SW_DOWNLOAD				"Download"
#define MSG_SW_CLOSE				"Schließen"
#define MSG_SW_CLEAR				"Löschen"
#define MSG_SW_STOP					"Stop"

#define MSG_IDLE                    "Bereit."
#define MSG_WF_RESULTS				"Ergebnisse: %1"
#define MSG_SEARCHING				"Suche nach: \"%1\"."

// Main Window

#define NAME						"Unizone (Deutsch)"		// Application Title

#define MSG_CSTATUS                 "Status:"
#define MSG_CSERVER                 "Server:"
#define MSG_CNICK                   "Nick:"

#define MSG_CONNECTING				"Verbinde mit Server %1."
#define MSG_CONNECTFAIL				"Verbindung mit Server fehlgeschlagen!"
#define MSG_CONNECTED				"Verbunden."
#define MSG_DISCONNECTED			"Vom Server getrennt."
#define MSG_NOTCONNECTED			"Nicht verbunden."
#define MSG_RESCAN					"Rescannen gemeinsamer Dateien..."
#define MSG_SCAN_ERR1				"Schon gescannt!"
#define MSG_NOTSHARING				"Gemeinsame Dateinutzung nicht aktiviert."
#define MSG_NONICK                  "Kein Nickname gesetzt."
#define MSG_NOMESSAGE				"Keine Nachricht zum senden."
#define MSG_AWAYMESSAGE				"Away message set to %1."
#define MSG_HEREMESSAGE				"Here message set to %1."
#define MSG_UNKNOWNCMD				"Unbekannter Befehl!"
#define MSG_NOUSERS                 "Kein(e) Benutzer anwesend."
#define MSG_NO_INDEX				"No index specified."
#define MSG_INVALID_INDEX			"Invalid index."
#define MSG_USERSNOTFOUND			"Benutzer nicht gefunden!"
#define MSG_PINGSENT				"Ping gesendet an Benutzer #%1 (a.k.a. <font color=\"%3\">%2</font>)."
#define MSG_NEWVERSION				" %1 ist erhältlich bei http://ozone-o3.net ."
#define MSG_SCANSHARES				"Scanning shares..."
#define MSG_SHARECOUNT				" %1 anteilige Dateie(n)."
#define MSG_NAMECHANGED				"Name geändert zu <font color=\"%2\">%1</font>."
#define MSG_UNKNOWN                 "Unbekannt"
#define MSG_USERHOST				"<font color=\"%3\">%1</font>'s IP address is %2."
#define MSG_NUM_USERS				"Anzahl eingeloggter Benutzer : %1\n"

#define MSG_UPTIME			"Uptime: %1"
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

#define MSG_HERE                    "here"
#define MSG_AWAY                    "away"
#define MSG_STATUS_IDLE				"bereit"
#define MSG_STATUS_BUSY				"busy"
#define MSG_AT_WORK					"at work"
#define MSG_AROUND					"around"
#define MSG_SLEEPING				"sleeping"

#define MSG_ACCEPT_THREAD_READY		"Bereit und lausche an Port %1."
#define MSG_ACCEPT_THREAD_FAILED	"Start der Befehlsfolge fehlgeschlagen!"
// #define MSG_LOG_CREATE_ERROR		"Erstellung der Log- Datei fehlgeschlagen."
#define MSG_WAIT_FOR_FST			"Warte auf Dateiscan..."

//
// Menus
//

// Menu bar

#define MSG_FILE                "&Datei"
#define MSG_AFILE               CTRL+Key_F
#define MSG_EDIT                "&Bearbeiten"
#define MSG_AEDIT               CTRL+Key_B
#define MSG_OPTIONS             "&Optionen"
#define MSG_AOPTIONS            CTRL+Key_O
#define MSG_HELP                "&Hilfe"
#define MSG_AHELP               CTRL+Key_H

// File menu

#define MSG_ABOUT               "&Über "
#define MSG_AABOUT              CTRL+Key_U

#define MSG_CONNECT             "&Verbinden"
#define MSG_ACONNECT            CTRL+SHIFT+Key_V
#define MSG_DISCONNECT          "&Trennen"
#define MSG_ADISCONNECT         CTRL+SHIFT+Key_T
#define MSG_OPEN_SHARED         "Öffne &Shared Ordner"
#define MSG_AOPEN_SHARED        CTRL+Key_S
#define MSG_OPEN_DOWNLOAD       "Öffne &Download Ordner"
#define MSG_AOPEN_DOWNLOAD      CTRL+Key_D
#define MSG_OPEN_LOGFOLDER      "Öffne &Log Ordner"
#define MSG_AOPEN_LOGFOLDER		CTRL+Key_L
#define MSG_CLEAR_CHATLOG       "Lösch&e Chat Log"
#define MSG_ACLEAR_CHATLOG      CTRL+Key_E
#define MSG_SEARCH              "Suche"
#define MSG_ASEARCH             CTRL+ALT+Key_S
#define MSG_EXIT                "&Beenden"
#define MSG_AEXIT               CTRL+Key_B

// Edit menu

#define MSG_PREFERENCES         "&Einstellungen"
#define MSG_APREFERENCES        CTRL+Shift+Key_E

// Preferences window
//

#define MSG_PR_PREFERENCES      "Einstellungen"

// Tab names

#define MSG_GENERAL             "General"
#define MSG_CONNECTION          "Verbindung"
#define MSG_DISPLAY             "Anzeige"
#define MSG_COLORS              "Farben"
#define MSG_STYLE               "Stil"
#define MSG_FILE_SHARING        "File Sharing"
#define MSG_URL_LAUNCHING       "URL launcher"
#define MSG_CHTTP_LAUNCHER      "HTTP launcher:"
#define MSG_CFTP_LAUNCHER       "FTP launcher:"
#define MSG_CMAILTO_LAUNCHER    "Mailto: launcher:"
#define MSG_THROTTLING          "Begrenzung"

// General

#define MSG_CAUTOAWAY           "Auto Abwesend:"
#define MSG_DISABLED            "Deaktiviert"
//#define MSG_MINUTES           "Minuten"
#define MSG_2_MINUTES           "2 Minuten"
#define MSG_5_MINUTES           "5 Minuten"
#define MSG_10_MINUTES          "10 Minuten"
#define MSG_15_MINUTES          "15 Minuten"
#define MSG_20_MINUTES          "20 Minuten"
#define MSG_30_MINUTES          "30 Minuten"
#define MSG_1_HOUR              "1 Stunde"
#define MSG_2_HOURS             "2 Stunden"
//#define MSG_HOUR              "Stunde"
//#define MSG_HOURS             "Stunden"
#define MSG_AUTOUPDATE          "Serverliste automatisch aktualisieren"
#define MSG_CHECK_NEW           "Nach neuer Version suchen"
#define MSG_LOGIN_ON_START      "Beim Start einloggen"
#define MSG_ENABLE_LOGGING      "Loggen aktiviert"
#define MSG_MULTI_COLOR_LISTVIEWS "Multi-color ListViews"

#define MSG_OK                  "OK"
#define MSG_CANCEL              "Abbruch"
#define MSG_TOGGLE_BLOCK		"(Un)block"

// Connection

#define MSG_CUPLOAD_BAND        "Upload Bandbreite:"
#define MSG_FIREWALLED          "Ich bin hinter einer Firewall"

// Display 

#define MSG_TIMESTAMPS          "Zeitmarken"
#define MSG_USEREVENTS          "Benutzer Aktivitäten"
#define MSG_UPLOADS             "Uploads"
#define MSG_CHAT                "Chat"
#define MSG_PRIVATE_MSGS        "Private Nachrichten"
#define MSG_INFO_MSGS           "Informationen"
#define MSG_WARNING_MSGS        "Warnungen"
#define MSG_ERROR_MSGS          "Fehlermeldungen"
#define MSG_FLASH_WINDOW        "zeige Fenster wenn notwendig"
#define MSG_FLASH_PRIVATE       "zeige Privat Fenster"
#define MSG_FONT_SIZE           "Schriftgröße"

// Colors

#define MSG_CDESCRIPTION        "Beschreibung:"
#define MSG_CPREVIEW            "Vorschau:"
#define MSG_CHANGE              "Wechseln"
#define MSG_LOCAL_NAME          "Localer Name"
#define MSG_HLOCAL_NAME         "Das ist die Farbe Deines Benutzernamens."
#define MSG_REMOTE_NAME         "Anderer Name"
#define MSG_HREMOTE_NAME        "Das ist die Farbe der anderen Benutzernamen."
#define MSG_REGULAR_TEXT        "Normaler Text"
#define MSG_HREGULAR_TEXT       "Das ist die Farbe des gesendeten Textes aller Benutzer."
#define MSG_SYSTEM_TEXT         "System Text"
#define MSG_HSYSTEM_TEXT        "Das ist die Farbe von \"System\"."
#define MSG_PING_TEXT           "Ping Text"
#define MSG_HPING_TEXT          "Die Farbe einer Ping- Antwort."
#define MSG_ERROR_TEXT          "Fehler Text"
#define MSG_HERROR_TEXT         "Das ist die Farbe von \"Error\"."
#define MSG_ERRORMSG_TEXT       "Fehlermeldung Text"
#define MSG_HERRORMSG_TEXT      "Das ist die Farbe einer Fehlermeldung."
#define MSG_PRIVATE_TEXT        "Privat Text"
#define MSG_HPRIVATE_TEXT       "Das ist die Farbe privaten Textes."
#define MSG_ACTION_TEXT         "Aktion Text"
#define MSG_HACTION_TEXT        "Das ist die Farbe der \"Aktion\"."
#define MSG_URL_TEXT            "URL Text"
#define MSG_HURL_TEXT           "Das ist die Farbe von URLs."
#define MSG_NAME_SAID_TEXT      "'Name gesagt' Text"
#define MSG_HNAME_SAID_TEXT     "Die Farbe Deines Benutzernamens, wenn ihn jemand im Hauptchat gesagt hat."
#define MSG_HSTYLE              "Der gewählte Style wird sofort bei der Auswahl übernommen."

// File Sharing

#define MSG_FS_ENABLED          "Datei- Tausch aktiviert?"
#define MSG_BINKYNUKE			"Block binkies?"
#define MSG_BLOCK_DISCONNECTED	"Block disconnected users?"
#define MSG_CFS_MAXUP           "Maximal gleichzeitige Uploads:"
#define MSG_CFS_MAXDOWN         "Maximal gleichzeitige Downloads:"

// Throttling

#define MSG_CCHAT                       "Chat:"
#define MSG_TH_UPLOADS                  "Uploads (pro upload):"
#define MSG_TH_DOWNLOADS                "Downloads (pro download):"
#define MSG_TH_BLOCKED					"Uploads (per blocked):"
#define MSG_UNLIMITED                   "Unbegrenzt"
#define MSG_NO_LIMIT                    "Kein Limit"
#define MSG_BYTES                       "bytes"

//
// Nick List
//

#define MSG_NL_NAME                     "Name"
// ID is not translated Yet
#define MSG_NL_STATUS                   "Status"
#define MSG_NL_FILES                    "Dateien"
#define MSG_NL_CONNECTION               "Verbindung"
#define MSG_NL_LOAD                     "Lade"
#define MSG_NL_CLIENT                   "Client"

#define MSG_NL_PRIVATECHAT              "Privater Chat mit %1"
#define MSG_NL_LISTALLFILES             "Liste aller Dateien"
#define MSG_NL_GETIPADDR                "Bekomme IP Addresse"
#define MSG_NL_GETADDRINFO				"Get Address Info"
#define MSG_REMOVE                      "Entfernen"

//
// Format Strings
//

#define MSG_WF_USERCONNECTED		"Benutzer #%1 ist jetzt verbunden."
#define MSG_WF_USERDISCONNECTED		"Benutzer #%1 (alias <font color=\"%3\">%2</font>) hat die Verbindung getrennt."
#define MSG_WF_USERNAMECHANGENO		"Benutzer #%1 ist bekannt als <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED		"Benutzer #%1 (alias <font color=\"%4\">%2</font>) heisst jetzt <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE		"Benutzer #%1 (alias <font color=\"%4\">%2</font>) ist jetzt %3."
#define MSG_WF_USERSTATUSCHANGE2	"Benutzer #%1 ist jetzt %2."
#define MSG_WF_STATUSCHANGED		"Sie sind jetzt %1."

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>System:</b> </font>"
#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">Ping zurück in %3 millisekunden (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (Uptime: %3, Logged on for %4)</font>"
#define MSG_WF_ERROR            "<font color=\"%1\" size=\"%2\"><b>Fehler:</b></font> "
#define MSG_WF_WARNING			"<font color=\"%1\" size=\"%2\"><b>Warning:</b></font> "
#define MSG_WF_ACTION           "<font color=\"%1\" size=\"%2\"><b>Aktion:</b></font> "
#define MSG_WF_GOTPINGED        "<font color=\"%1\" size=\"%2\">Benutzer #%3 (alias <font color=\"%5\">%4</font>) hat dich gepingt.</font>"



//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define MSG_TX_STATUS                   MSG_NL_STATUS
#endif

#define MSG_TX_FILENAME                 "Dateiname"
#define MSG_TX_RECEIVED                 "Empfangen"
#define MSG_TX_TOTAL                    "Gesamt"
#define MSG_TX_ETA						"ETA" 
#define MSG_TX_RATE						"Rate"
#define MSG_TX_USER						"Benutzer" 
#define MSG_TX_SENT						"Gesendet" 
#define MSG_TX_QUEUE					"Queue"
#define MSG_TX_THROTTLE					"Throttle"
#define MSG_TX_MOVEUP					"Move Up"
#define MSG_TX_MOVEDOWN					"Move Down"
#define MSG_TX_CAPTION					"Datei Transfer"

#define MSG_TX_ISDOWNLOADING	"%1 is downloading %2."
#define MSG_TX_HASFINISHED		"%1 has finished downloading %2."
#define MSG_TX_FINISHED			"Finished downloading %2 from %1."

#endif