// Portuguese strings

#ifndef LANG_PORTUGUESE_H
#define LANG_PORTUGUESE_H

//
// Search Window
//

#define MSG_SW_CAPTION			"Pesquisar"

#define MSG_SW_CSEARCH			"Pesquisa:"
#define MSG_SW_FILENAME			"Nome do Arquivo"
#define MSG_SW_FILESIZE			"Tamanho do Arquivo"
#define MSG_SW_FILETYPE			"Tipo"
#define MSG_SW_MODIFIED			"Modificado"
#define MSG_SW_PATH				"Caminho"
#define MSG_SW_USER				"Usuário"
#define MSG_SW_DOWNLOAD			"Download"
#define MSG_SW_CLOSE			"Fechar"
#define MSG_SW_CLEAR			"Limpar"
#define MSG_SW_STOP				"Parar"

#define MSG_IDLE				"Ocioso."
#define MSG_WF_RESULTS			"Resultados: %1"
#define MSG_SEARCHING			"Procurando por: \"%1\"."
#define MSG_WAIT_FOR_FST		"Esperando a varredura de arquivos terminar..."

// Main Window

#define NAME				"Unizone (Português)"				// Application Title

#define MSG_CSTATUS			"Status:"
#define MSG_CSERVER			"Servidor:"
#define MSG_CNICK			"Apelido:"

#define MSG_CONNECTING      "Conectando no servidor %1."
#define MSG_CONNECTFAIL		"Conexão com o servidor falhou!"
#define MSG_CONNECTED       "Conectado."
#define MSG_DISCONNECTED	"Desconectado do servidor."
#define MSG_NOTCONNECTED    "Desconectado."
#define MSG_RESCAN          "Procurando por arquivos compartilhados..."
#define MSG_SCAN_ERR1		"Já está procurando!"
#define MSG_NOTSHARING		"Compartilhamento de arquivos desabilitado."
#define MSG_NONICK			"Nenhum apelido foi dado."
#define MSG_NOMESSAGE		"Nenhuma mensagem a enviar."
#define MSG_AWAYMESSAGE		"Mensagem de ausncia configurada para %1."
#define MSG_HEREMESSAGE		"Mensagem de presena configurada para %1."
#define MSG_UNKNOWNCMD		"Comando desconhecido!"
#define MSG_NOUSERS			"Não foi informado um usuário."
#define MSG_NO_INDEX		"No index specified."
#define MSG_INVALID_INDEX	"Invalid index."
#define MSG_USERSNOTFOUND   "Usurio(s) não encontrado(s)!"
#define MSG_PINGSENT		"Ping enviado ao usuário #%1 (também conhecido por <font color=\"%3\">%2</font>)."
#define MSG_NEWVERSION		" %1 est disponvel em http://www.raasu.org/tools/windows/."
#define MSG_SCANSHARES		"Scanning shares..."
#define MSG_SHARECOUNT		"Compartilhando %1 arquivo(s)."
#define MSG_NAMECHANGED		"Nome mudado para <font color=\"%2\">%1</font>."
#define MSG_UNKNOWN			"Desconhecido"
#define MSG_USERHOST		"O IP de <font color=\"%3\">%1</font> é %2."
#define MSG_NUM_USERS		"Usuários conectados: %1\n"

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

#define MSG_HERE			"aqui"
#define MSG_AWAY			"fora"
#define MSG_STATUS_IDLE		"ocioso"
#define MSG_STATUS_BUSY		"busy"
#define MSG_AT_WORK			"at work"
#define MSG_AROUND			"around"
#define MSG_SLEEPING		"sleeping"

//
// Menus
//

// Menu bar

#define MSG_FILE			"&Arquivo"
//#define MSG_AFILE			ALT+Key_A
#define MSG_EDIT			"&Editar"
//#define MSG_AEDIT			CTRL+Key_E
//#define MSG_OPTIONS			"&Opões"
//#define MSG_AOPTIONS		CTRL+Key_O
#define MSG_HELP			"A&juda"
//#define MSG_AHELP			CTRL+Key_J

// File menu

#define MSG_CONNECT			"&Conectar"
#define MSG_ACONNECT		CTRL+SHIFT+Key_C
#define MSG_DISCONNECT		"&Desconectar"
#define MSG_ADISCONNECT		CTRL+SHIFT+Key_D
#define MSG_OPEN_SHARED		"Abrir Pasta &Shared"
#define MSG_AOPEN_SHARED	CTRL+Key_S
#define MSG_OPEN_DOWNLOAD	"Abrir Pasta &Downloads"
#define MSG_AOPEN_DOWNLOAD	CTRL+Key_D
#define MSG_OPEN_LOGFOLDER	"Abrir Pasta &Logs"
#define MSG_AOPEN_LOGFOLDER CTRL+Key_L
#define MSG_CLEAR_CHATLOG	"Limpar &Histórico do Chat"
#define MSG_ACLEAR_CHATLOG	CTRL+Key_H
#define MSG_SEARCH			"Pesquisar"
#define MSG_ASEARCH			CTRL+ALT+Key_P
#define MSG_EXIT			"&Fechar"
#define MSG_AEXIT			CTRL+Key_F

// Edit menu

#define MSG_PREFERENCES		"&Preferências"
#define MSG_APREFERENCES	CTRL+Key_P

// Help menu

#define MSG_ABOUT			"Sob&re "
#define MSG_AABOUT			Key_F12

// Preferences window
//

#define MSG_PR_PREFERENCES		"Preferncias"

// Tab names

#define MSG_GENERAL				"Geral"
#define MSG_CONNECTION			"Conexão"
#define MSG_DISPLAY				"Mostrar"
#define MSG_COLORS				"Cores"
#define MSG_STYLE				"Estilo"
#define MSG_FILE_SHARING		"Compartilhamento de arquivos"
#define MSG_URL_LAUNCHING		"Ver URL com"
#define MSG_CHTTP_LAUNCHER		"Ver HTTP com:"
#define MSG_CFTP_LAUNCHER		"Ver FTP com:"
#define MSG_CMAILTO_LAUNCHER	"Abrir Mailto: com:"
#define MSG_THROTTLING			"Estrangulamento"

// General

#define MSG_CAUTOAWAY		"Ausência Automática:"
#define MSG_DISABLED		"Desabilitado"
#define MSG_2_MINUTES		"2 Minutos"
#define MSG_5_MINUTES		"5 Minutos"
#define MSG_10_MINUTES		"10 Minutos"
#define MSG_15_MINUTES		"15 Minutos"
#define MSG_20_MINUTES		"20 Minutos"
#define MSG_30_MINUTES		"30 Minutos"
#define MSG_1_HOUR			"1 Hora"
#define MSG_2_HOURS			"2 Horas"
#define MSG_AUTOUPDATE		"Atualizar Lista de Servidores"
#define MSG_CHECK_NEW		"Checar Novas Verses"
#define MSG_LOGIN_ON_START	"Conectar ao Iniciar"
#define MSG_ENABLE_LOGGING	"Log ativado"
#define MSG_MULTI_COLOR_LISTVIEWS "Multi-color ListViews"

#define MSG_OK				"OK"
#define MSG_CANCEL			"Cancelar"
#define MSG_TOGGLE_BLOCK	"(Un)block"

// Connection

#define MSG_CUPLOAD_BAND	"Banda de Transferência (upload):"
#define MSG_FIREWALLED		"Estou atrás de um Firewall"

// Display 

#define MSG_TIMESTAMPS		"Hora do Evento"
#define MSG_USEREVENTS		"Eventos de Usuário"
#define MSG_UPLOADS			"Uploads"
#define MSG_CHAT			"Conversação"
#define MSG_PRIVATE_MSGS	"Mensagens Privadas"
#define MSG_INFO_MSGS		"Mensagens Informativas"
#define MSG_WARNING_MSGS	"Mensagens de Aviso"
#define MSG_ERROR_MSGS		"Mensagens de Erro"
#define MSG_FLASH_WINDOW	"Piscar Janela Quando Chamado"
#define MSG_FLASH_PRIVATE	"Piscar Janelas do Chat Privado"
#define MSG_FONT_SIZE		"Tamanho da Fonte"

// Colors

#define MSG_CDESCRIPTION	"Descrião:"
#define MSG_CPREVIEW		"Prever:"
#define MSG_CHANGE			"Mudar"
#define MSG_LOCAL_NAME		"Nome Local"
#define MSG_HLOCAL_NAME		"Esta é a cor do seu nome de usuário."
#define MSG_REMOTE_NAME		"Nome Remoto"
#define MSG_HREMOTE_NAME	"Esta é a cor do nome de outros usuários."
#define MSG_REGULAR_TEXT	"Texto Normal"
#define MSG_HREGULAR_TEXT	"Esta  a cor do texto enviado por voc e outros usurios."
#define MSG_SYSTEM_TEXT		"Texto do Sistema"
#define MSG_HSYSTEM_TEXT	"Esta é a cor do \"Sistema\"."
#define MSG_PING_TEXT		"Texto do Ping"
#define MSG_HPING_TEXT		"Esta é a cor do texto na resposta do ping."
#define MSG_ERROR_TEXT		"Texto de Erro"
#define MSG_HERROR_TEXT		"Esta é a cor do \"Erro\"."
#define MSG_ERRORMSG_TEXT	"Texto da Mensagem de Erro"
#define MSG_HERRORMSG_TEXT	"Esta é a cor do texto nas mensagens de erro."
#define MSG_PRIVATE_TEXT	"Texto Privado"
#define MSG_HPRIVATE_TEXT	"Esta  a cor do texto privado."
#define MSG_ACTION_TEXT		"Texto da Ação"
#define MSG_HACTION_TEXT	"Esta é a cor da \"Ação\"."
#define MSG_URL_TEXT		"Texto da URL"
#define MSG_HURL_TEXT		"Esta  a cor das URLs."
#define MSG_NAME_SAID_TEXT	"Texto 'Nome Pronunciado'"
#define MSG_HNAME_SAID_TEXT "Esta é a cor da cor do seu nome de usurio no texto quando alguém pronunciar seu nome no chat principal."
#define MSG_HSTYLE			"O estilo escolhido vai ser ativado assim que for selecionado."

// File Sharing

#define MSG_FS_ENABLED		"Compartilhamento de Arquivos Habilitado?"
#define MSG_BINKYNUKE		"Block binkies?"
#define MSG_BLOCK_DISCONNECTED "Block disconnected users?"
#define MSG_CFS_MAXUP		"Máximo de Uploads Simultâneos:"
#define MSG_CFS_MAXDOWN		"Máximo de Downloads Simultâneos:"

// Throttling

#define MSG_CCHAT				"Conversaço:"
#define MSG_TH_UPLOADS			"Uploads (por upload)"
#define MSG_TH_DOWNLOADS		"Downloads (por download)"
#define MSG_TH_BLOCKED			"Uploads (per blocked)"
#define MSG_UNLIMITED			"Ilimitado"
#define MSG_NO_LIMIT			"Sem Limite"
#define MSG_BYTES				"bytes"

//
// Nick List
//

#define MSG_NL_NAME				"Nome"
// ID is not translated Yet
#define MSG_NL_STATUS			"Status"
#define MSG_NL_FILES			"Arquivos"
#define MSG_NL_CONNECTION		"Conexão"
#define MSG_NL_LOAD				"Carga"
#define MSG_NL_CLIENT			"Cliente"

#define MSG_NL_PRIVATECHAT		"Conversação Privada Com %1"
#define MSG_NL_LISTALLFILES		"Listar Todos os Arquivos"
#define MSG_NL_GETIPADDR		"Pegar Endereço IP"
#define MSG_NL_GETADDRINFO		"Get Address Info"
#define MSG_REMOVE				"Remove" // TODO: Translate this ;)
//
// Format Strings
//

#define MSG_WF_USERCONNECTED		"Usuário #%1 está conectado."
#define MSG_WF_USERDISCONNECTED		"Usuário #%1 (t.c.c. <font color=\"%3\">%2</font>) desconectou-se."
#define MSG_WF_USERNAMECHANGENO		"Usuário #%1 é conhecido como <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED		"Usuário #%1 (t.c.c. <font color=\"%4\">%2</font>) agora é conhecido como <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE		"Usuário #%1 (t.c.c. <font color=\"%4\">%2</font>) agora está %3."
#define MSG_WF_USERSTATUSCHANGE2	"Usuário #%1 agora está %2."
#define MSG_WF_STATUSCHANGED		"Você agora está %1."

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>Sistema:</b> </font>"
#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">O Ping retornou em %3 milisegundos (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (Uptime: %3, conectado por %4)</font>"
#define MSG_WF_ERROR			"<font color=\"%1\" size=\"%2\"><b>Erro:</b></font> "
#define MSG_WF_WARNING			"<font color=\"%1\" size=\"%2\"><b>Warning:</b></font> "
#define MSG_WF_ACTION           "<font color=\"%1\" size=\"%2\"><b>Ação:</b></font> "
#define MSG_WF_GOTPINGED		"<font color=\"%1\" size=\"%2\">Usuário #%3 (t.c.c. <font color=\"%5\">%4</font>) pingou voc.</font>"



//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define MSG_TX_STATUS			MSG_NL_STATUS
#endif

#define	MSG_TX_FILENAME			"Nome do arquivo"
#define MSG_TX_RECEIVED			"Recebido"
#define	MSG_TX_TOTAL			"Total"
#define	MSG_TX_RATE				"Taxa"
#define MSG_TX_ETA				"ETA"
#define MSG_TX_USER				"Usuário"
#define MSG_TX_SENT				"Enviado"
#define MSG_TX_QUEUE			"Queue"
#define MSG_TX_THROTTLE			"Throttle"
#define MSG_TX_MOVEUP			"Move Up"
#define MSG_TX_MOVEDOWN			"Move Down"
#define MSG_TX_CAPTION			"Transferências de arquivo"

#define MSG_TX_ISDOWNLOADING	"%1 is downloading %2."
#define MSG_TX_HASFINISHED		"%1 has finished downloading %2."
#define MSG_TX_FINISHED			"Finished downloading %2 from %1."

#endif