#ifndef LANG_FRENCH_H
#define LANG_FRENCH_H

//
// Search Window
//

#define MSG_SW_CAPTION			"Rechercher"

#define MSG_SW_CSEARCH        	"Chercher:"
#define MSG_SW_FILENAME       	"Nom du Fichier"
#define MSG_SW_FILESIZE       	"Taille du Fichier"
#define MSG_SW_FILETYPE       	"Type de Fichier"
#define MSG_SW_MODIFIED       	"Modifié"
#define MSG_SW_PATH           	"Chemin"
#define MSG_SW_USER           	"Utilisateur"
#define MSG_SW_DOWNLOAD       	"Télécharger"
#define MSG_SW_CLOSE          	"Fermer"
#define MSG_SW_CLEAR          	"Effacer"
#define MSG_SW_STOP          	"Arréter"

#define MSG_IDLE				"libre."
#define MSG_WF_RESULTS			"Resultats: %1"
#define MSG_SEARCHING			"Recherche en cours de: \"%1\"."
#define MSG_WAIT_FOR_FST		"En attente de la fin du thread de recherche fichier..."

// Main Window

#define NAME			"Unizone (Français)"	// Application Title

#define MSG_CSTATUS		"Statut :"
#define MSG_CSERVER		"Serveur :"
#define MSG_CNICK		"Surnom :"

#define MSG_CONNECTING   	"Connexion au serveur %1 en cours."
#define MSG_CONNECTFAIL		"Connexion au serveur échouée!"
#define MSG_CONNECTED       "Connecté."
#define MSG_DISCONNECTED	"Déconnecté du serveur."
#define MSG_NOTCONNECTED    "Non connecté."
#define MSG_RESCAN          "Re-scan des fichiers partagés..."
#define MSG_SCAN_ERR1		"Scan en cours!"
#define MSG_NOTSHARING		"Partage de fichier désactivé."
#define MSG_NONICK			"Pas de surnom passé."
#define MSG_NOMESSAGE		"Pas de message à envoyer."
#define MSG_AWAYMESSAGE		"Message Absent fixé à %1."
#define MSG_HEREMESSAGE		"Message Présent fixé à %1."
#define MSG_UNKNOWNCMD		"Commande inconnue!"
#define MSG_NOUSERS			"Aucun utilisateur passé."
#define MSG_NO_INDEX		"No index specified."
#define MSG_INVALID_INDEX	"Invalid index."
#define MSG_USERSNOTFOUND   "Utilisateur(s) introuvable(s)!"
#define MSG_PINGSENT		"Ping envoyé à l'utilisateur #%1 (a.k.a. <font color=\"%3\">%2</font>)."
#define MSG_NEWVERSION		" %1 est disponible sur http://www.raasu.org/tools/windows/."
#define MSG_SCANSHARES		"Scanning shares..."
#define MSG_SHARECOUNT		"Partageant %1 file(s)."
#define MSG_NAMECHANGED		"Nom changé à <font color=\"%2\">%1</font>."
#define MSG_UNKNOWN			"Inconnu"
#define MSG_USERHOST		"L'adresse IP de <font color=\"%3\">%1</font> est %2."
#define MSG_NUM_USERS		"Nombre d'utilisateur connecté : %1\n"

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

#define MSG_HERE			"présent"
#define MSG_AWAY			"absent"
#define MSG_STATUS_IDLE		"libre"
#define MSG_STATUS_BUSY		"busy"
#define MSG_AT_WORK			"at work"
#define MSG_AROUND			"around"
#define MSG_SLEEPING		"sleeping"

//
// Menus
//

// Menu bar

#define MSG_FILE		"&Fichier"
//#define MSG_AFILE		CTRL+Key_F
#define MSG_EDIT		"&Edition"
//#define MSG_AEDIT		CTRL+Key_E
//#define MSG_OPTIONS		"&Options"
//#define MSG_AOPTIONS	CTRL+Key_O
#define MSG_HELP		"&Aide"
//#define MSG_AHELP		CTRL+Key_H

// File menu

#define MSG_CONNECT				"Se &Connecter"
#define MSG_ACONNECT			CTRL+SHIFT+Key_C
#define MSG_DISCONNECT			"Se &Déconnecter"
#define MSG_ADISCONNECT			CTRL+SHIFT+Key_D
#define MSG_OPEN_SHARED			"Ouvrir le do&Ssier partagé"
#define MSG_AOPEN_SHARED		CTRL+Key_S
#define MSG_OPEN_DOWNLOAD		"Ouvrir le dossier des &Téléchargements"
#define MSG_AOPEN_DOWNLOAD		CTRL+Key_T
#define MSG_OPEN_LOGFOLDER		"Ouvrir le dossier des &Logs"
#define MSG_AOPEN_LOGFOLDER 	CTRL+Key_L
#define MSG_CLEAR_CHATLOG		"Effacer le l&Og de discussion"
#define MSG_ACLEAR_CHATLOG		CTRL+Key_O
#define MSG_SEARCH				"Rechercher"
#define MSG_ASEARCH				CTRL+ALT+Key_R
#define MSG_EXIT				"&Quitter"
#define MSG_AEXIT				CTRL+Key_Q

// Edit menu

#define MSG_PREFERENCES		"&Preférences"
#define MSG_APREFERENCES	CTRL+Key_P

// Help menu

#define MSG_ABOUT		"&A propos "
#define MSG_AABOUT		Key_F12

// Preferences window
//

#define MSG_PR_PREFERENCES	"Préférences"

// Tab names

#define MSG_GENERAL				"Général"
#define MSG_CONNECTION			"Connexion"
#define MSG_DISPLAY				"Affichage"
#define MSG_COLORS				"Couleurs"
#define MSG_STYLE				"Style"
#define MSG_FILE_SHARING		"Partage de fichier"
#define MSG_URL_LAUNCHING		"Lancement d'une URL"
#define MSG_CHTTP_LAUNCHER		"Lancement HTTP :"
#define MSG_CFTP_LAUNCHER		"Lancement FTP :"
#define MSG_CMAILTO_LAUNCHER	"Lancement Mailto :"
#define MSG_THROTTLING			"Contrôle des flux"

// General

#define MSG_CAUTOAWAY		"Absence Auto :"
#define MSG_DISABLED		"Désactivée"
#define MSG_2_MINUTES		"2 Minutes"
#define MSG_5_MINUTES		"5 Minutes"
#define MSG_10_MINUTES		"10 Minutes"
#define MSG_15_MINUTES		"15 Minutes"
#define MSG_20_MINUTES		"20 Minutes"
#define MSG_30_MINUTES		"30 Minutes"
#define MSG_1_HOUR			"1 Heure"
#define MSG_2_HOURS			"2 Heures"
#define MSG_AUTOUPDATE		"MAJ auto. liste serveurs"
#define MSG_CHECK_NEW		"Vérifier si nouvelle version"
#define MSG_LOGIN_ON_START	"Se connecter au lancement"
#define MSG_ENABLE_LOGGING	"Activer les Traces"
#define MSG_MULTI_COLOR_LISTVIEWS "Multi-color ListViews"

#define MSG_OK				"OK"
#define MSG_CANCEL			"Annuler"
#define MSG_TOGGLE_BLOCK	"(Un)block"

// Connection

#define MSG_CUPLOAD_BAND	"Bande passante de téléchargement :"
#define MSG_FIREWALLED		"Je suis protégé par un coupe-feu"

// Display 

#define MSG_TIMESTAMPS		"Info Temps"
#define MSG_USEREVENTS		"Evénements utilisateur"
#define MSG_UPLOADS			"Uploads"
#define MSG_CHAT			"Discussion"
#define MSG_PRIVATE_MSGS	"Messages privés"
#define MSG_INFO_MSGS		"Messages d'info."
#define MSG_WARNING_MSGS	"Messages d'avertissement"
#define MSG_ERROR_MSGS		"Messages d'erreur"
#define MSG_FLASH_WINDOW	"Flasher la fenêtre si nommé"
#define MSG_FLASH_PRIVATE	"Flasher la fenêtre privée"
#define MSG_FONT_SIZE		"Taille de la police"

// Colors

#define MSG_CDESCRIPTION		"Description :"
#define MSG_CPREVIEW			"Aperçu :"
#define MSG_CHANGE				"Changer"
#define MSG_LOCAL_NAME			"Nom local"
#define MSG_HLOCAL_NAME			"C'est la couleur de votre nom d'utilisateur."
#define MSG_REMOTE_NAME			"Nom distant"
#define MSG_HREMOTE_NAME		"C'est la couleur des autres noms d'utilisateur."
#define MSG_REGULAR_TEXT		"Texte standard"
#define MSG_HREGULAR_TEXT		"C'est la couleur du texte saisi par vous et les autres utilisateurs."
#define MSG_SYSTEM_TEXT			"Texte système"
#define MSG_HSYSTEM_TEXT		"C'est la couleur du texte de \"System\"."
#define MSG_PING_TEXT			"Texte ping"
#define MSG_HPING_TEXT			"C'est la couleur du texte d'une réponse à un ping."
#define MSG_ERROR_TEXT			"Texte erreur"
#define MSG_HERROR_TEXT			"C'est la couleur du texte de \"Error\"."
#define MSG_ERRORMSG_TEXT		"Texte message d'erreur"
#define MSG_HERRORMSG_TEXT		"C'est la couleur du texte des messages d'erreur."
#define MSG_PRIVATE_TEXT		"Texte privé"
#define MSG_HPRIVATE_TEXT		"C'est la couleur du texte privé."
#define MSG_ACTION_TEXT			"Texte action"
#define MSG_HACTION_TEXT		"C'est la couleur du texte de \"Action\"."
#define MSG_URL_TEXT			"Texte URL"
#define MSG_HURL_TEXT			"C'est la couleur du texte d'URLs."
#define MSG_NAME_SAID_TEXT		"Texte 'Nom dit'"
#define MSG_HNAME_SAID_TEXT 	"C'est la couleur du texte de votre nom d'utilisateur nommé dans un message de la discussion principale."
#define MSG_HSTYLE				"Le style sélectionné sera appliqué dès qu'il est choisi."

// File Sharing

#define MSG_FS_ENABLED		"Partage de fichier activé ?"
#define MSG_BINKYNUKE		"Block binkies?"
#define MSG_BLOCK_DISCONNECTED "Block disconnected users?"
#define MSG_CFS_MAXUP		"Uploads simultanés max :"
#define MSG_CFS_MAXDOWN		"Downloads simultanés max :"

// Throttling

#define MSG_CCHAT			"Discussion :"
#define MSG_TH_UPLOADS		"Uploads (par upload) :"
#define MSG_TH_DOWNLOADS	"Downloads (par download) :"
#define MSG_TH_BLOCKED		"Uploads (per blocked) :"
#define MSG_UNLIMITED		"Illimité"
#define MSG_NO_LIMIT		"Pas de Limite"
#define MSG_BYTES			"octets"

//
// Nick List
//

#define MSG_NL_NAME			"Nom"
// ID is not translated Yet
#define MSG_NL_STATUS		"Statut"
#define MSG_NL_FILES		"Fichiers"
#define MSG_NL_CONNECTION	"Connexion"
#define MSG_NL_LOAD			"Charger"
#define MSG_NL_CLIENT		"Client"

#define MSG_NL_PRIVATECHAT	"Discussion Privée avec %1"
#define MSG_NL_LISTALLFILES	"Lister tous les Fichiers"
#define MSG_NL_GETIPADDR	"Obtenir l'Adresse IP"
#define MSG_NL_GETADDRINFO	"Get Address Info"
#define MSG_REMOVE			"Supprimer"

//
// Format Strings
//

#define MSG_WF_USERCONNECTED		"Utilisateur #%1 est maintenant connecté."
#define MSG_WF_USERDISCONNECTED		"Utilisateur #%1 (a.k.a. <font color=\"%3\">%2</font>) est déconnecté."
#define MSG_WF_USERNAMECHANGENO		"Utilisateur #%1 est connu maintenant comme <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED		"Utilisateur #%1 (a.k.a. <font color=\"%4\">%2</font>) est connu maintenant comme <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE		"Utilisateur #%1 (a.k.a. <font color=\"%4\">%2</font>) est maintenant %3."
#define MSG_WF_USERSTATUSCHANGE2	"Ulilisateur #%1 est maintenant %2."
#define MSG_WF_STATUSCHANGED		"Vous êtes maintenant %1."

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>Système :</b> </font>"
#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">Ping retourné en %3 millisecondes (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (Uptime: %3, connecté sur %4)</font>"
#define MSG_WF_ERROR			"<font color=\"%1\" size=\"%2\"><b>Erreur :</b></font> "
#define MSG_WF_WARNING			"<font color=\"%1\" size=\"%2\"><b>Warning :</b></font> "
#define MSG_WF_ACTION           "<font color=\"%1\" size=\"%2\"><b>Action :</b></font> "
#define MSG_WF_GOTPINGED		"<font color=\"%1\" size=\"%2\">Utilisateur #%3 (a.k.a. <font color=\"%5\">%4</font>) vous a pingué.</font>"



//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define MSG_TX_STATUS		MSG_NL_STATUS
#endif

#define	MSG_TX_FILENAME		"Nom du fichier"
#define MSG_TX_RECEIVED		"Reçu"
#define	MSG_TX_TOTAL		"Total"
#define	MSG_TX_RATE			"Taux"
#define MSG_TX_ETA			"ETA"
#define MSG_TX_USER			"Utilisateur"
#define MSG_TX_SENT			"Envoyé"
#define MSG_TX_QUEUE		"Queue"
#define MSG_TX_THROTTLE		"Throttle"
#define MSG_TX_MOVEUP		"Move Up"
#define MSG_TX_MOVEDOWN		"Move Down"
#define MSG_TX_CAPTION		"Transferts de Fichier"

#define MSG_TX_ISDOWNLOADING	"%1 is downloading %2."
#define MSG_TX_HASFINISHED		"%1 has finished downloading %2."
#define MSG_TX_FINISHED			"Finished downloading %2 from %1."

#endif