#include "parser.h"
#include "tokenizer.h"
#include "util.h"							// for endsWith()

static const unsigned int MAX_BUFFER_SIZE = 262144;	// 256 kB

unsigned int
ParseBufferSize()
{
	return MAX_BUFFER_SIZE;
}

QString
TrimBuffer(const QString &txt)
{
	// Check for too big text
	unsigned int n2 = 0;
	if (txt.length() > MAX_BUFFER_SIZE)
	{
		n2 = txt.length() - MAX_BUFFER_SIZE;		// Start of text
		unsigned int n3 = n2;						// Position of next line break after buffer truncation
		
		// Find next line break
		while (n3 < txt.length())
		{
			if (txt.mid(n3, 4) == "<br>")
			{
				n2 = n3 + 4;
				break;
			}
			else if (txt[n3] == "\t")
			{
				n2 = n3 + 1;
				break;
			}
			n3++;
		}
	}

	/*
	 * 
	 * Remove any extra line breaks from the start of buffer
	 *
	 */

	while (n2 < txt.length())
	{
		if (txt.mid(n2, 4) == "<br>")
		{
			n2 += 4;
		}
		else if (txt[n2] == "\t")
		{
			n2++;
		}
		else
		{
			break; // Found start of actual text
		}
	}
	if (n2 < txt.length())
		return (n2 == 0) ? txt : txt.mid(n2);
	else
		return QString::null;
}

QString
ParseForShown(const QString & txt)
{
	if (!txt.isEmpty())
	{
		QString out = ParseForShownAux(txt);

		// <postmaster@raasu.org> 20030721 -- Strip off trailing line break, we don't need it
		if (endsWith(out, "<br>"))
			out.truncate(out.length() - 4);
		return out;
	}
	return txt;
}

QString
ParseForShownAux(const QString &txt)
{
	if (txt.contains('\t'))
	{
		int s = ((txt.length() / 256) + 1);
		s *= 256;
		QString out((QChar *) NULL, s);	// Text after processing linefeeds and TABs, preallocate

		QStringTokenizer tok(txt, "\t");
		QString token;
		while ((token = tok.GetNextToken()) != QString::null)
		{
			if (!out.isEmpty())
				out += "<br>";
			out += token;
		}
		return out;
	}
	return txt;
}

