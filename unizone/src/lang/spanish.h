// Spanish Strings

#ifndef LANG_SPANISH_H
#define LANG_SPANISH_H

//
// Search Window
//

#define MSG_SW_CAPTION			"Buscar"

#define MSG_SW_CSEARCH			"Buscar:"
#define MSG_SW_FILENAME			"Nombre"
#define MSG_SW_FILESIZE			"Tamaño"
#define MSG_SW_FILETYPE			"Tipo"
#define MSG_SW_MODIFIED			"Modificado"
#define MSG_SW_PATH				"Directorio"
#define MSG_SW_USER				"Usuario"
#define MSG_SW_DOWNLOAD			"Descarga"
#define MSG_SW_CLOSE			"Cerrar"
#define MSG_SW_CLEAR			"Limpiar"
#define MSG_SW_STOP				"Detener"

#define MSG_IDLE				"Idle."
#define MSG_WF_RESULTS			"Resultados: %1"
#define MSG_SEARCHING			"Buscando: \"%1\"."
#define MSG_WAIT_FOR_FST		"Esperando escaneo para terminar..."

// Main Window

#define NAME				"Unizone (Español)"				// Application Title

#define MSG_CSTATUS			"Estado:"
#define MSG_CSERVER			"Servidor:"
#define MSG_CNICK			"Nick:"

#define MSG_CONNECTING      "Conectando al servidor %1."
#define MSG_CONNECTFAIL		"¡ La conexión al servidor ha fallado !"
#define MSG_CONNECTED       "Conectado."
#define MSG_DISCONNECTED	"Desconectado del servidor."
#define MSG_NOTCONNECTED    "No conectado."
#define MSG_RESCAN          "Reescaneando archivos compartidos..."
#define MSG_SCAN_ERR1		"¡ Todavía escaneando !"
#define MSG_NOTSHARING		"Compartir archivos no activado."
#define MSG_NONICK			"Nick no introducido."
#define MSG_NOMESSAGE		"No hay mensaje que enviar."
#define MSG_AWAYMESSAGE		"Mensaje de ausencia: %1."
#define MSG_HEREMESSAGE		"Mensaje de regreso: %1."
#define MSG_UNKNOWNCMD		"¡ Comando desconocido !"
#define MSG_NOUSERS			"Usuario no introducido."
#define MSG_NO_INDEX		"Índice no especificado."
#define MSG_INVALID_INDEX	"Índice inválido."
#define MSG_USERSNOTFOUND   "¡ Usuario(s) no encontrado !"
#define MSG_PINGSENT		"Ping enviado al usuario #%1 (a.k.a. <font color=\"%3\">%2</font>)."
#define MSG_NEWVERSION		" %1 está disponible en http://www.raasu.org/tools/windows/."
#define MSG_SCANSHARES		"Escaneando directorios compartidos..."
#define MSG_SHARECOUNT		"Compartiendo %1 archivo(s)."
#define MSG_NAMECHANGED		"Nombre cambiado a <font color=\"%2\">%1</font>."
#define MSG_UNKNOWN			"Desconocido"
#define MSG_USERHOST		"La direcciín IP de <font color=\"%3\">%1</font> es %2."
#define MSG_NUM_USERS		"Número de usuarios conectados: %1\n"

#define MSG_UPTIME			"Uptime: %1"
#define MSG_WEEK			"semana"
#define MSG_WEEKS			"semanas"
#define MSG_DAY				"día"
#define MSG_DAYS			"días"
#define MSG_HOUR			"hora"
#define MSG_HOURS			"horas"
#define MSG_MINUTE			"minuto"
#define MSG_MINUTES			"minutos"
#define MSG_SECOND			"segundo"
#define MSG_SECONDS			"segundos"
#define MSG_AND				" y "

#define MSG_HERE			"aquí"
#define MSG_AWAY			"ausente"
#define MSG_STATUS_IDLE		"idle"
#define MSG_STATUS_BUSY		"ocupado"
#define MSG_AT_WORK			"trabajando"
#define MSG_AROUND			"por aquí"
#define MSG_SLEEPING		"durmiendo"
//
// Menus
//

// Menu bar

#define MSG_FILE			"&Archivo"
//#define MSG_AFILE			CTRL+Key_A
#define MSG_EDIT			"&Edición"
//#define MSG_AEDIT			CTRL+Key_E
//#define MSG_OPTIONS			"&Opciones"
//#define MSG_AOPTIONS		CTRL+Key_O
#define MSG_HELP			"&Ayuda"
//#define MSG_AHELP			CTRL+Key_H

// File menu

#define MSG_CONNECT			"&Conectar"
#define MSG_ACONNECT		CTRL+SHIFT+Key_C
#define MSG_DISCONNECT		"&Desconectar"
#define MSG_ADISCONNECT		CTRL+SHIFT+Key_D
#define MSG_OPEN_SHARED		"Abrir Directorio &Compartido"
#define MSG_AOPEN_SHARED	CTRL+Key_S
#define MSG_OPEN_DOWNLOAD	"Abrir Directorio &Descargas"
#define MSG_AOPEN_DOWNLOAD	CTRL+Key_D
#define MSG_OPEN_LOGFOLDER	"Abrir Directorio &Logs"
#define MSG_AOPEN_LOGFOLDER CTRL+Key_L
#define MSG_CLEAR_CHATLOG	"Cl&ear Chat Log"
#define MSG_ACLEAR_CHATLOG	CTRL+SHIFT+Key_E
#define MSG_SEARCH			"&Buscar"
#define MSG_ASEARCH			CTRL+ALT+Key_B
#define MSG_EXIT			"Salir"
#define MSG_AEXIT			Key_F12

// Edit menu

#define MSG_PREFERENCES		"&Preferencias"
#define MSG_APREFERENCES	CTRL+Key_P

// Help menu

#define MSG_ABOUT			"&Sobre "
#define MSG_AABOUT			Key_F12

// Preferences window
//

#define MSG_PR_PREFERENCES		"Preferencias"

// Tab names

#define MSG_GENERAL				"General"
#define MSG_CONNECTION			"Conexión"
#define MSG_DISPLAY				"Mostrar"
#define MSG_COLORS				"Colores"
#define MSG_STYLE				"Estilo"
#define MSG_FILE_SHARING		"Compartir archivos"
#define MSG_URL_LAUNCHING		"Configuración URL"
#define MSG_CHTTP_LAUNCHER		"Cliente HTTP:"
#define MSG_CFTP_LAUNCHER		"Cliente FTP:"
#define MSG_CMAILTO_LAUNCHER	"Cliente e-mail:"
#define MSG_THROTTLING			"Throttling"

// General

#define MSG_CAUTOAWAY		"Auto Ausentarse:"
#define MSG_DISABLED		"Desactivado"
#define MSG_2_MINUTES		"2 Minutos"
#define MSG_5_MINUTES		"5 Minutos"
#define MSG_10_MINUTES		"10 Minutos"
#define MSG_15_MINUTES		"15 Minutos"
#define MSG_20_MINUTES		"20 Minutos"
#define MSG_30_MINUTES		"30 Minutos"
#define MSG_1_HOUR			"1 Hora"
#define MSG_2_HOURS			"2 Horas"
#define MSG_AUTOUPDATE		"Auto Actualizar Lista de Servidores"
#define MSG_CHECK_NEW		"Comprobar Nuevas Versiones"
#define MSG_LOGIN_ON_START	"Conectar al inicio"
#define MSG_ENABLE_LOGGING	"Activar Logging"
#define MSG_MULTI_COLOR_LISTVIEWS "Lista de Vistas Multi-color"

#define MSG_OK				"Aceptar"
#define MSG_CANCEL			"Cancelar"
#define MSG_TOGGLE_BLOCK	"(Des)bloquear"

// Connection

#define MSG_CUPLOAD_BAND	"Ancho de banda de Subida"
#define MSG_FIREWALLED		"Estoy detrás de un Firewall"

// Display 

#define MSG_TIMESTAMPS		"Time Stamps"
#define MSG_USEREVENTS		"Eventos de Usuario"
#define MSG_UPLOADS			"Subidas"
#define MSG_CHAT			"Chat"
#define MSG_PRIVATE_MSGS	"Mensajes Privados"
#define MSG_INFO_MSGS		"Mensajes Informativos"
#define MSG_WARNING_MSGS	"Mensajes de Aviso"
#define MSG_ERROR_MSGS		"Mensajes de Error"
#define MSG_FLASH_WINDOW	"Parpadear ventana al mencionar su nick"
#define MSG_FLASH_PRIVATE	"Parpadear ventanas privadas"
#define MSG_FONT_SIZE		"Tamaño de letra"

// Colors

#define MSG_CDESCRIPTION	"Descripción:"
#define MSG_CPREVIEW		"Previsualizar:"
#define MSG_CHANGE			"Cambiar"
#define MSG_LOCAL_NAME		"Nombre local"
#define MSG_HLOCAL_NAME		"Este es el color para su nombre de usuario."
#define MSG_REMOTE_NAME		"Nombre remoto"
#define MSG_HREMOTE_NAME	"Este es el color para otros nombres de usuario."
#define MSG_REGULAR_TEXT	"Texto normal"
#define MSG_HREGULAR_TEXT	"Este es el color de texto enviado por usted y otros usuarios."
#define MSG_SYSTEM_TEXT		"Texto del sistema"
#define MSG_HSYSTEM_TEXT	"Este es el color de \"System\"."
#define MSG_PING_TEXT		"Texto de ping"
#define MSG_HPING_TEXT		"Este es el color de texto de la respuesta a ping."
#define MSG_ERROR_TEXT		"Texto de error"
#define MSG_HERROR_TEXT		"Este es el color de \"Error\"."
#define MSG_ERRORMSG_TEXT	"Texto de mensajes de error"
#define MSG_HERRORMSG_TEXT	"Este es el color de texto en mensajes de error."
#define MSG_PRIVATE_TEXT	"Texto privado"
#define MSG_HPRIVATE_TEXT	"Este es el color de texto privado."
#define MSG_ACTION_TEXT		"Texto de acciónAction Text"
#define MSG_HACTION_TEXT	"Este es el color de texto de \"Action\"."
#define MSG_URL_TEXT		"Texto de URL"
#define MSG_HURL_TEXT		"Este es el color de texto de URLs."
#define MSG_NAME_SAID_TEXT	"Texto de 'Nombre Dijo'"
#define MSG_HNAME_SAID_TEXT "Este es el color de su nombre de usuario cuando alguien le menciona el el chat principal."
#define MSG_HSTYLE			"El estilo seleccionado será aplicado tan pronto como sea seleccionado."

// File Sharing

#define MSG_FS_ENABLED		"¿ Activar compartir archivos ?"
#define MSG_BINKYNUKE		"¿ Bloquear 'binkies'?"
#define MSG_BLOCK_DISCONNECTED "¿ Bloquear usuarios desconectados ?"
#define MSG_CFS_MAXUP		"Límite de uploads simultáneos:"
#define MSG_CFS_MAXDOWN		"Límite de descargas simultáneas:"

// Throttling

#define MSG_CCHAT				"Chat:"
#define MSG_TH_UPLOADS			"Uploads (por upload):"
#define MSG_TH_DOWNLOADS		"Descargas (por Descarga):"
#define MSG_TH_BLOCKED			"Uploads (per bloqueado):"
#define MSG_UNLIMITED			"Ilimitado"
#define MSG_NO_LIMIT			"Sin Límite"
#define MSG_BYTES				"bytes"

//
// Nick List
//

#define MSG_NL_NAME				"Nombre"
// ID is not translated Yet
#define MSG_NL_STATUS			"Estado"
#define MSG_NL_FILES			"Archivos"
#define MSG_NL_CONNECTION		"Conexión"
#define MSG_NL_LOAD				"Uso"
#define MSG_NL_CLIENT			"Cliente"

#define MSG_NL_PRIVATECHAT		"Chat privado con %1"
#define MSG_NL_LISTALLFILES		"Listar todos los archivos"
#define MSG_NL_GETIPADDR		"Dirección IP"
#define MSG_NL_GETADDRINFO		"Información sobre la Dirección"
#define MSG_REMOVE				"Eliminar"

//
// Format Strings
//

#define MSG_WF_USERCONNECTED		"El usuario #%1 ha conectado."
#define MSG_WF_USERDISCONNECTED		"El usuario #%1 (a.k.a. <font color=\"%3\">%2</font>) ha desconectado."
#define MSG_WF_USERNAMECHANGENO		"El usuario #%1 es conocido como <font color=\"%3\">%2</font>."
#define MSG_WF_USERNAMECHANGED		"El usuario #%1 (a.k.a. <font color=\"%4\">%2</font>) se renombra a <font color=\"%5\">%3</font>."
#define MSG_WF_USERSTATUSCHANGE		"El usuario #%1 (a.k.a. <font color=\"%4\">%2</font>) está %3."
#define MSG_WF_USERSTATUSCHANGE2	"El usuario #%1 está %2."
#define MSG_WF_STATUSCHANGED		"Usted está %1."

#define MSG_WF_SYSTEMTEXT       "<font color=\"%1\" size=\"%2\"><b>Sistema::</b> </font>"
#define MSG_WF_PINGTEXT         "<font color=\"%1\" size=\"%2\">Respuesta a Ping en %3 milisegundos (%4)</font>"
#define MSG_WF_PINGUPTIME       "<font color=\"%1\" size=\"%2\"> (Uptime: %3, conectado por %4)</font>"
#define MSG_WF_ERROR			"<font color=\"%1\" size=\"%2\"><b>Error:</b></font> "
#define MSG_WF_WARNING			"<font color=\"%1\" size=\"%2\"><b>Aviso:</b></font> "
#define MSG_WF_ACTION           "<font color=\"%1\" size=\"%2\"><b>Acción:</b></font> "
#define MSG_WF_GOTPINGED		"<font color=\"%1\" size=\"%2\">El usuario #%3 (a.k.a. <font color=\"%5\">%4</font>) le ha hecho ping.</font>"



//
// Transfer Window
//

#ifdef MSG_NL_STATUS
#define MSG_TX_STATUS			MSG_NL_STATUS
#endif

#define	MSG_TX_FILENAME			"Nombre"
#define MSG_TX_RECEIVED			"Recibido"
#define	MSG_TX_TOTAL			"Total"
#define	MSG_TX_RATE				"Rate"
#define MSG_TX_ETA				"ETA"
#define MSG_TX_USER				"Usuario"
#define MSG_TX_SENT				"Enviado"
#define MSG_TX_QUEUE			"Cola"
#define MSG_TX_THROTTLE			"Throttle"
#define MSG_TX_MOVEUP			"Mover Arriba"
#define MSG_TX_MOVEDOWN			"Mover Abajo"
#define MSG_TX_CAPTION			"Transferencia de Archivos"

#define MSG_TX_ISDOWNLOADING	"%1 está descargando %2."
#define MSG_TX_HASFINISHED		"%1 ha acabado de descargar %2."
#define MSG_TX_FINISHED			"Descarga finalizada %2 de %1."

#endif