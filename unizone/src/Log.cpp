#include "Log.h"
#include "util.h"
#include "debugimpl.h"

#include <time.h>
#include <string.h>

#include <util/String.h>
using muscle::String;

#include <qregexp.h>

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
		fFile->writeBlock(logClose.Cstr(), logClose.Length());
		CloseFile(fFile);
	}
}

void
WLog::Create(LogType type, const QString &name)
{
	int counter = 0;
	time_t currentTime = time(NULL);
	QString lt(asctime(localtime(&currentTime)));
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
	
	lt.truncate(lt.find("\n"));
	switch (type)
	{
	case LogMain: 
		break;
	case LogPrivate:
		lt = lt.prepend("private_");
		break;
	case LogChannel:
		lt = lt.prepend("_");
		lt = lt.prepend(name);
		lt = lt.prepend("channel_");
		break;
	}
	lt = lt.replace(QRegExp(" "), "_");
	lt = FixFileName(lt);
	lt = lt.prepend("logs/");
	fullPath = lt + ".html";
	
	// Delete old log
	if (InitCheck())
		Close();

	// create a new one
	fFile = new QFile(fullPath);
	CHECK_PTR(fFile);
	while (fFile->exists())	// to avoid name conflicts (very possible!)
	{
		counter++;
		fullPath = lt + "_";
		fullPath += QString::number(counter);
		fullPath += ".html";
		fFile->setName(fullPath);
	}
	if (fFile)
	{
		if (fFile->open(IO_WriteOnly))
		{
			QCString out = prepend.utf8();
			fFile->writeBlock(out, out.length());
			fFile->flush();
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

void
WLog::LogString(const char * txt)
{
	String text(txt);
	text += "<br>";
	if (InitCheck())
	{
		fFile->writeBlock(text.Cstr(), text.Length());
		fFile->flush();
	}
}

void
WLog::LogString(const QString & txt)
{
	QString out = txt;
	out += "<br>";
	QCString ctxt = out.utf8();
	if (InitCheck())
	{
		fFile->writeBlock(ctxt, ctxt.length());
		fFile->flush();
	}
}
