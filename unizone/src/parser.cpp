#include "parser.h"

#define MAX_BUFFER_SIZE 262144		// 256 kB

int
ParseBufferSize()
{
	return MAX_BUFFER_SIZE;
}

void
TrimBuffer(QString &txt)
{
	// Check for too big text
	if (txt.length() > MAX_BUFFER_SIZE)
	{
		int n2 = txt.length() - MAX_BUFFER_SIZE;	// Start of text
		int n3 = n2;								// Position of next line break after buffer truncation
		
		// Find next line break
		while (n3 < (int) txt.length())
		{
			if (txt.mid(n3, 4) == "<br>")
			{
				n2 = n3 + 4;
				break;
			}
			else if (txt.mid(n3,1) == "\t")
			{
				n2 = n3 + 1;
				break;
			}
			n3++;
		}
		txt = txt.mid(n2);
	}

	// Remove any extra line breaks from the start of buffer
	//

	while (txt.length() > 0)
	{
		if (txt.left(4) == "<br>")
		{
			txt = txt.mid(4);
		}
		else if (txt.left(1) == "\t")
		{
			txt = txt.mid(1);
		}
		else
		{
			break; // Found start of actual text
		}
	}
}

QString
ParseForShown(const QString & txt)
{
	if (txt.length() > 0)
	{
		QString out = ParseForShownAux(txt);

		// <postmaster@raasu.org> 20030721 -- Strip off trailing line break, we don't need it
		if (out.right(4) == "<br>")
			out.truncate(out.length() - 4);
		return out;
	}
	return txt;
}

QString
ParseForShownAux(const QString &txt)
{
	int n;				// Position of next TAB before text to be included
	int m = 0;			// Position of next TAB after text to be included

	n = txt.find('\t');

	if (n >= 0)
	{
		int s = ((txt.length() / 256) + 1);
		s *= 256;
		QString out((QChar *) NULL, s);	// Text after processing linefeeds and TABs, preallocate

		// copy everything before first TAB
		out = txt.left(n);

		// skip the TAB ;)
		n++;

		while (n < (int) txt.length())
		{
			out += "<br>";
			m = txt.find('\t', n);
			if (m > n)
			{
				out += txt.mid(n, m - n);
				n = m + 1;
			}
			else
			{
				// no more TAB characters
				out += txt.mid(n);
				break;
			}
		}
		return out;
	}
	return txt;
}

