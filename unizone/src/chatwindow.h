#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "util/String.h"

#include "htmlview.h"

#include <QString.h>
#include <QWidget.h>

#ifdef WIN32
#pragma warning(disable: 4786)
#endif

using namespace muscle;

class WSettings;

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

	// see if we were named...
	bool NameSaid(QString & msg);	// msg will be syntaxed if needed

#ifdef WIN32
	HWND GetHandle() { return fWinHandle; }
#endif

protected:

	void BeforeShown();
	void GotShown(const QString & msg);

	WHTMLView * fChatText;

	bool fScrollDown;			// do we need to scroll the view down after an insertion?

#ifdef WIN32					// if the OS is Windows,
	HWND fWinHandle;			// handle to our window for flashing
#endif

private:
	bool NameSaid2(const QString &sname, QString & msg, unsigned long index = 0); // Private version for recursing
	void CheckScrollState();
	void UpdateTextView();		// moves the stuff in the chat screen so that the latest stuff is displayed


	ChatType _type;

	int fScrollX, fScrollY;

};
#endif