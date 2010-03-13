#include "Log.h"
#include "util.h"
#include "debugimpl.h"
#include "wfile.h"

#include <time.h>
#include <string.h>

#include "util/String.h"
#include <QByteArray>

using muscle::String;

#include <qregexp.h>
#include <qfile.h>

WLog::WLog()
{
	fFile = NULL;
}

WLog::~WLog()
{
	if (InitCheck())	// are we still valid? 
		Close();		// we shouldn't be, so close the file
}

void
WLog::Close()
{
	String logClose = "</BODY></HTML>";
	if (fFile)
	{
		LogString(logClose.Cstr(), false);
		CloseFile(fFile);
	}
}

void
WLog::Create(LogType type)
{
	Create(type, QString::null);
}

void
WLog::Create(LogType type, const QString &name)
{
	int counter = 0;
	time_t currentTime = time(NULL);
	QString lt = QString::fromLocal8Bit( ctime(&currentTime) );
	lt.truncate(lt.find("\n"));
	QString fullPath;

	QString prepend = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\"><TITLE>Log for: ";
	prepend += lt;
	if (type == LogPrivate)
		prepend += " (Private)";
	else if (type == LogChannel)
	{
		prepend += " (Channel &quot;";
		prepend += name;
		prepend += "&quot;)";
	}
	prepend += "</TITLE></HEAD><BODY BGCOLOR=\"#FFFFFF\">";
	
	switch (type)
	{
	case LogMain: 
		break;
	case LogPrivate:
		lt.prepend("private_");
		break;
	case LogChannel:
		lt.prepend("_");
		lt.prepend(name);
		lt.prepend("channel_");
		break;
	}
	lt.replace(QRegExp(" "), "_");
	lt = FixFileName(lt);
	lt.prepend("logs/");
	fullPath = lt + ".html";
	
	// Delete old log
	if (InitCheck())
		Close();

	// create a new one
	while (WFile::Exists(fullPath))	// to avoid name conflicts (very possible!)
	{
		counter++;
		fullPath = lt + "_";
		fullPath += QString::number(counter);
		fullPath += ".html";
	}
	if (!fFile)
		fFile = new WFile();
	if (fFile)
	{
		if (fFile->Open(fullPath, QIODevice::WriteOnly))
		{
			LogString(prepend, false);
		}
		else
		{
			delete fFile;
			fFile = NULL;
		}
	}
}

bool
WLog::InitCheck() const
{
	return fFile ? true : false;
}

const char * br = "<br>";
#ifdef WIN32
const char * lend = "\r\n";
#else
const char * lend = "\n";
#endif

void
WLog::LogString(const char * txt, bool brk)
{
	if (InitCheck())
	{
		fFile->WriteBlock(txt, strlen(txt));
		if (brk)
			fFile->WriteBlock(br, strlen(br));
		fFile->WriteBlock(lend, strlen(lend));
		fFile->Flush();
	}
}

void
WLog::LogString(const QString & txt, bool brk)
{
	QByteArray ctxt = txt.utf8();
	LogString((const char *) ctxt, brk);
}
