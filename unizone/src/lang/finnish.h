// Finnish strings

#ifndef LANG_FINNISH_H
#define LANG_FINNISH_H


//
// Search Window
//

#define MSG_IDLE				"Vapaa."
#define MSG_WF_RESULTS			"Tuloksia: %1"
#define MSG_SEARCHING			"Etsii: \"%1\"."
#define MSG_WAIT_FOR_FST		"Odotetaan, ett‰ tiedostojenhakus‰ie on valmis..."

#define MSG_SW_CSEARCH			"Etsi:"
#define MSG_SW_FILENAME			"Nimi"
#define MSG_SW_FILESIZE			"Koko"
#define MSG_SW_FILETYPE			"Tyyppi"
#define MSG_SW_MODIFIED			"Muokattu"
#define MSG_SW_PATH				"Polku"
#define MSG_SW_USER				"K‰ytt‰j‰"
#define MSG_SW_DOWNLOAD			"Lataa"
#define MSG_SW_CLOSE			"Sulje"
#define MSG_SW_CLEAR			"Tyhjenn‰"
#define MSG_SW_STOP				"Pys‰yt‰"

//
// Main window 
//

#define NAME				"Unizone (Finnish)"

#define MSG_CSTATUS			"Tila:"
#define MSG_CSERVER			"Palvelin:"
#define MSG_CNICK			"Nimi:"
#define MSG_CONNECTING      "Yhdistet‰‰n palvelimelle %1."
#define MSG_CONNECTFAIL		"Yhteytt‰ palvelimelle ei voitu muodostaa!"
#define MSG_CONNECTED       "Yhdistetty."
#define MSG_DISCONNECTED	"Yhteys on katkennut."
#define MSG_NOTCONNECTED    "Ei yhdistetty."
#define MSG_RESCAN          "Tutkitaan uudelleen jaetut tiedostot..."
#define MSG_SCAN_ERR1		"Tutkinta on jo k‰ynniss‰!"
#define MSG_NOTSHARING		"Tiedostojen jako ei ole p‰‰ll‰."
#define MSG_NONICK			"Et antanut nime‰."
#define MSG_NOMESSAGE		"Et antanut viesti‰ l‰hetett‰v‰ksi."
#define MSG_AWAYMESSAGE		"Poissa-viestisi on nyt %1."
#define MSG_HEREMESSAGE		"Paikalla-viestisi on nyt %1."
#define MSG_UNKNOWNCMD		"Tuntematon komento!"
#define MSG_NOUSERS			"Et antanut k‰ytt‰ji‰."
#define MSG_NO_INDEX		"Et antanut indeksi‰."
#define MSG_INVALID_INDEX	"Ep‰kelpo indeksi."
#define MSG_USERSNOTFOUND   "K‰ytt‰j‰‰(-ji‰) ei lˆytynyt!"
#define MSG_PINGSENT		"Ping l‰hetetty k‰ytt‰j‰lle #%1, eli <font color=\"%3\">%2</font>."
#define MSG_NEWVERSION		" %1 on ilmestynyt osoitteessa http://www.raasu.org/tools/windows/."
#define MSG_SCANSHARES		"Tutkitaan jaot..."
#define MSG_SHARECOUNT		"%1 tiedosto(a) jaettuna."
#define MSG_NAMECHANGED		"Nimesi on nyt <font color=\"%2\">%1</font>."
#define	MSG_UNKNOWN			"Tuntematon"
#define MSG_USERHOST		"<font color=\"%3\">%1</font>:n IP-osoite on %2."
#define MSG_NUM_USERS		"K‰ytt‰ji‰ linjoilla: %1\n"

#define MSG_HERE			"t‰‰ll‰"
#define MSG_AWAY			"poissa"
#define MSG_STATUS_IDLE		"laiskottelemassa"
#define MSG_STATUS_BUSY		"kiireinen"
#define MSG_AT_WORK			"tˆiss‰"
#define MSG_AROUND			"ymp‰rill‰"

//
// Menus
//

// Menu bar

#define MSG_FILE			"&Tiedosto"
#define MSG_AFILE			CTRL+Key_T
#define MSG_EDIT			"&Muokkaa"
#define MSG_AEDIT			CTRL+Key_M
#define MSG_OPTIONS			"&Ominaisuudet"
#define MSG_AOPTIONS		CTRL+Key_O
#define MSG_HELP			"&Ohje"
#define MSG_AHELP			CTRL+Key_O

// File menu

#define MSG_CONNECT			"&Yhdist‰"
#define MSG_ACONNECT		CTRL+SHIFT+Key_Y
#define MSG_DISCONNECT		"&Katkaise"
#define MSG_ADISCONNECT		CTRL+SHIFT+Key_K
#define MSG_OPEN_SHARED		"Avaa &Jaetut-kansio"
#define MSG_AOPEN_SHARED	CTRL+Key_J
#define MSG_OPEN_DOWNLOAD	"Avaa &Vastaanotetut-kansio"
#define MSG_AOPEN_DOWNLOAD	CTRL+Key_V
#define MSG_OPEN_LOGFOLDER	"Avaa &Lokikansio"
#define MSG_AOPEN_LOGFOLDER CTRL+Key_L
#define MSG_CLEAR_CHATLOG	"Tyhjenn‰ &Keskusteluloki"
#define MSG_ACLEAR_CHATLOG	CTRL+Key_K
#define MSG_SEARCH			"Etsi"
#define MSG_ASEARCH			CTRL+ALT+Key_S
#define MSG_ABOUT			"&Tietoja "
#define MSG_AABOUT			CTRL+Key_T
#define MSG_EXIT			"&Poistu"
#define MSG_AEXIT			CTRL+Key_P

// Edit menu

#define MSG_PREFERENCES		"A&setukset"
#define MSG_APREFERENCES	CTRL+Key_S


//
// Preferences
//

#define MSG_PR_PREFERENCES		"Asetukset"

// Tab names

#define MSG_GENERAL				"Yleiset"
#define MSG_CONNECTION			"Yhteys"
#define MSG_DISPLAY				"N‰yttˆ"
#define MSG_COLORS				"V‰rit"
#define MSG_STYLE				"Tyyli"
#define MSG_FILE_SHARING		"Tiedostojen jako"
#define MSG_URL_LAUNCHING		"URLien avaus"
#define MSG_CHTTP_LAUNCHER		"HTTP-aukaisija:"
#define MSG_CFTP_LAUNCHER		"FTP-aukaisija:"
#define MSG_CMAILTO_LAUNCHER	"Mailto:-aukaisija:"
#define MSG_THROTTLING			"Rajoitukset"

// General

#define MSG_CAUTOAWAY		"Automaattisesti pois:"
#define MSG_DISABLED		"Ei p‰‰ll‰"
#define MSG_2_MINUTES		"2 Minuuttia"
#define MSG_5_MINUTES		"5 Minuuttia"
#define MSG_10_MINUTES		"10 Minuuttia"
#define MSG_15_MINUTES		"15 Minuuttia"
#define MSG_20_MINUTES		"20 Minuuttia"
#define MSG_30_MINUTES		"30 Minuuttia"
#define MSG_1_HOUR			"1 Tunti"
#define MSG_2_HOURS			"2 Tuntia"
#define MSG_AUTOUPDATE		"Palvelinlistan automaattip‰ivitys"
#define MSG_CHECK_NEW		"Tarkista uusi versio"
#define MSG_LOGIN_ON_START	"Kirjaudu alussa"
#define MSG_ENABLE_LOGGING	"Loki p‰‰ll‰"
#define MSG_MULTI_COLOR_LISTVIEWS "Moniv‰riset listat"

#define MSG_OK				"OK"
#define MSG_CANCEL			"Peruuta"

// Connection

#define MSG_CUPLOAD_BAND	"L‰hetysten kaista:"
#define MSG_FIREWALLED		"Palomuurin takana"

#define MSG_TIMESTAMPS		"Aikaleimat"
#define MSG_USEREVENTS		"K‰ytt‰j‰n tapahtumat"
#define MSG_UPLOADS			"L‰hetykset"
#define MSG_CHAT			"Keskustelu"
#define MSG_PRIVATE_MSGS	"Yksityisviestit"
#define MSG_INFO_MSGS		"Tiedoitukset"
#define MSG_WARNING_MSGS	"Varoitusviestit"
#define MSG_ERROR_MSGS		"Virheviestit"
#define MSG_FLASH_WINDOW	"V‰l‰yt‰ ikkunaa mainittaessa"
#define MSG_FLASH_PRIVATE	"V‰l‰yt‰ yksityisikkunoita" 
#define MSG_FONT_SIZE		"Kirjasinkoko"

// Colors

#define MSG_CDESCRIPTION	"Kuvaus:"
#define MSG_CPREVIEW		"Esikatselu:"
#define MSG_CHANGE			"Vaihda"
#define MSG_LOCAL_NAME		"Oma nimi"
#define MSG_HLOCAL_NAME		"Oman k‰ytt‰j‰nimen v‰ri."
#define MSG_REMOTE_NAME		"Muiden nimi"
#define MSG_HREMOTE_NAME	"Muiden nimien v‰ri."
#define MSG_REGULAR_TEXT	"Tavallinen teksti"
#define MSG_HREGULAR_TEXT	"Sinun ja muiden k‰ytt‰jien tekstin v‰ri."
#define MSG_SYSTEM_TEXT		"J‰rjestelm‰-teksti"
#define MSG_HSYSTEM_TEXT	"T‰m‰ on tekstin \"J‰rjestelm‰\" v‰ri."
#define MSG_PING_TEXT		"Ping-teksti"
#define MSG_HPING_TEXT		"Ping-vastauksen v‰ri."
#define MSG_ERROR_TEXT		"Virhe-teksti"
#define MSG_HERROR_TEXT		"Tekstin \"Virhe\" v‰ri."
#define MSG_ERRORMSG_TEXT	"Virheviestin teksti"
#define MSG_HERRORMSG_TEXT	"Virheviestien tekstin v‰ri."
#define MSG_PRIVATE_TEXT	"Yksityinen-teksti"
#define MSG_HPRIVATE_TEXT	"Yksityisen tekstin v‰ri."
#define MSG_ACTION_TEXT		"Toiminta Teksti"
#define MSG_HACTION_TEXT	"Tekstin \"Toiminta\" v‰ri."
#define MSG_URL_TEXT		"URL-teksti"
#define MSG_HURL_TEXT		"URLien v‰ri."
#define MSG_NAME_SAID_TEXT	"'Nimi Sanottu' Teksti"
#define MSG_HNAME_SAID_TEXT "T‰m‰ on sinun k‰ytt‰j‰nimesi v‰ri, kun joku sanoo sen p‰‰keskustelussa."
#define MSG_HSTYLE			"Valittu tyyli otetaan k‰yttˆˆn heti, kun se on valittu."

// File Sharing

#define MSG_FS_ENABLED		"Tiedostojen jako p‰‰ll‰?"
#define MSG_BINKYNUKE		"Est‰ binkyjen imuroinnit?"
#define MSG_BLOCK_DISCONNECTED "Est‰ poistuneiden imuroinnit?"
#define MSG_CFS_MAXUP		"Samanaikaisia l‰hetyksi‰ enint‰‰n:"
#define MSG_CFS_MAXDOWN		"Samanaikaisia vastaanottoja enint‰‰n:"

// Throttling

#define MSG_CCHAT			"Keskustelu:"
#define MSG_TH_UPLOADS		"Per l‰hetys:"
#define MSG_TH_DOWNLOADS	"Per vastaanotto:"
#define MSG_TH_BLOCKED		"Per estetty:"
#define MSG_UNLIMITED		"Rajaton"
#define MSG_NO_LIMIT		"Ei rajaa"
#define MSG_BYTES			"tavua"

//
// Nick List
//

#define MSG_NL_NAME           "Nimi"
// ID is not translated Yet
#define MSG_NL_STATUS         "Tila"	
#define MSG_NL_FILES          "Tiedostoja"
#define MSG_NL_CONNECTION     "Yhteys"
#define MSG_NL_LOAD           "Kuorma"
#define MSG_NL_CLIENT         "Ohjelma"

#define MSG_NL_PRIVATECHAT		"Yksityinen keskustelu %1:n kanssa"
#define MSG_NL_LISTALLFILES		"N‰yt‰ kaikki tiedostot"
#define MSG_NL_GETIPADDR		"Hae IP-osoite"
#define MSG_REMOVE				"Poista"

// Format Strings
//

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>J‰rjestelm‰:</b> </font>"
#define MSG_WF_USERCONNECTED    "K‰ytt‰j‰ #%1 on nyt linjoilla."
#define MSG_WF_USERDISCONNECTED "K‰ytt‰j‰ #%1, eli <font color=\"%3\">%2</font>, on poistunut."
#define MSG_WF_USERNAMECHANGENO "K‰ytt‰j‰ #%1 on nyt nimelt‰‰n <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED  "K‰ytt‰j‰ #%1, eli <font color=\"%4\">%2</font>, on nyt nimelt‰‰n <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE "K‰ytt‰j‰ #%1, eli <font color=\"%4\">%2</font>, on nyt %3."
#define MSG_WF_STATUSCHANGED	"Olet nyt %1."

#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">Ping tuli takaisin %3 millisekunnissa (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (K‰ynniss‰: %3, Kirjautuneena %4)</font>"
#define MSG_WF_ERROR			"<font color=\"%1\" size=\"%2\"><b>Virhe:</b></font> "
#define MSG_WF_ACTION			"<font color=\"%1\" size=\"%2\"><b>Toiminta:</b></font> "
#define MSG_WF_GOTPINGED		"<font color=\"%1\" size=\"%2\">K‰ytt‰j‰ #%3, eli <font color=\"%5\">%4</font>, pingasi sinua.</font>"

//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define	MSG_TX_STATUS			MSG_NL_STATUS
#endif

#define	MSG_TX_FILENAME			"Tiedosto"
#define MSG_TX_RECEIVED			"Vastaanotettu"
#define	MSG_TX_TOTAL			"Koko"
#define	MSG_TX_RATE				"Nopeus"
#define MSG_TX_ETA				"J‰ljell‰"
#define MSG_TX_USER				"K‰ytt‰j‰"
#define MSG_TX_SENT				"L‰hetetty"
#define MSG_TX_CAPTION			"Tiedostonsiirto"

#endif