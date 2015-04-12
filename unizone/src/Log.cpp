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
	QString lt, logtime;
#if __STDC_WANT_SECURE_LIB__
	char buf[26];
	ctime_s(buf, 26, &currentTime);
	logtime = QString::fromLocal8Bit(buf);
#else
	logtime = QString::fromLocal8Bit( ctime(&currentTime) );
#endif
	logtime.truncate(logtime.find("\n"));
	QString fullPath;

	QString prepend = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;charset=utf-8\"><TITLE>Log for: ";
	prepend += logtime;
	if (type == LogPrivate)
		prepend += " (Private)";
	else if (type == LogChannel)
	{
		prepend += QString(" (Channel &quot;%1&quot;)").arg(name);
	}
	prepend += "</TITLE></HEAD><BODY BGCOLOR=\"#FFFFFF\">";
	
	switch (type)
	{
	case LogMain:
		lt = logtime;
		break;
	case LogPrivate:
		lt = QString("private_%1").arg(logtime);
		break;
	case LogChannel:
		lt = QString("channel_%1_%2").arg(name, logtime);
		break;
	}
	lt.replace(QRegExp(" "), "_");
	lt = FixFileName(lt);
	fullPath = QString("logs/%1.html").arg(lt);
	
	// Delete old log
	if (InitCheck())
		Close();

	// create a new one
	while (WFile::Exists(fullPath))	// to avoid name conflicts (very possible!)
	{
		counter++;
		fullPath = QString("logs/%1_%2.html").arg(lt, QString::number(counter));
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
