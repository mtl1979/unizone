#include "Log.h"
#include "debugimpl.h"

#include <time.h>
#include <string.h>

#include <util/String.h>
using muscle::String;

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
		fFile->close();
		delete fFile;
		fFile = NULL;
	}
}

void
WLog::Create(bool priv)
{
	int counter = 0;
	time_t currentTime = time(NULL);
	String lt(asctime(localtime(&currentTime)));
	String fullPath;

	String prepend = "<HTML><HEAD><TITLE>Log for: ";
	prepend += lt;
	if (priv)
		prepend += " (Private)";
	prepend += "</TITLE></HEAD><BODY BGCOLOR=\"#FFFFFF\">";
	
	lt.Replace(" ", "_");
	lt.Replace(":", ".");
	lt = lt.Substring(0, "\n");
	if (priv)
		lt = lt.Prepend("private_");
	lt = lt.Prepend("logs/");
	fullPath = lt + ".html";
	
	// Delete old log
	if (InitCheck())
		Close();

	// create a new one
	fFile = new QFile(fullPath.Cstr());
	CHECK_PTR(fFile);
	while (fFile->exists())	// to avoid name conflicts (very possible!)
	{
		counter++;
		fullPath = lt + "_";
		fullPath << counter;
		fullPath += ".html";
		fFile->setName(fullPath.Cstr());
	}
	if (fFile)
	{
		if (fFile->open(IO_WriteOnly))
		{
			fFile->writeBlock(prepend.Cstr(), prepend.Length());
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
WLog::LogString(QString txt)
{
	txt += "<br>";
	if (InitCheck())
	{
		fFile->writeBlock(txt.latin1(), txt.length());
		fFile->flush();
	}
}
