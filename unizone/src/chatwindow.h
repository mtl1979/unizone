#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <q3listview.h>
#include <qstring.h>
#include <qwidget.h>

#include "util/String.h"

#include "htmlview.h"

using namespace muscle;

class WSettings;
class QListView;

typedef enum 
{
	MainType,
	PrivateType,
	ChannelType
} ChatType;

class ChatWindow
{
public:
	ChatWindow(ChatType type);
	virtual ~ChatWindow();

	WSettings *Settings();

	virtual void LogString(const QString & txt) = 0;
	virtual void LogString(const char * txt) = 0;
	virtual QWidget *Window() = 0;

	void Action(const QString & name, const QString & msg);
	void PrintText(const QString & str);
	void PrintSystem(const QString & msg);
	void PrintError(const QString & error);
	void PrintWarning(const QString & warning);

	void beep();
	void Clear();

	QString FormatNameSaid(const QString & msg); // Check and format message for Name Said...

protected:

	WHTMLView * fChatText;

	void InitUserList(Q3ListView * lv);

private:
	// see if we were named...
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed
	bool NameSaid2(const QString &sname, QString & msg, unsigned long index = 0); // Private version for recursing

	QString tr(const char *);

	ChatType _type;
	bool _inTable;
};
#endif
