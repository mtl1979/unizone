#include "parser.h"

#define MAX_BUFFER_SIZE 262144		// 256 kB

int
ParseBufferSize()
{
	return MAX_BUFFER_SIZE;
}

QString
ParseForShown(const QString & txt)
{
	// <postmaster@raasu.org> 20021005,20021128 -- Don't use latin1(), use QStringTokenizer ;)
	QString out;
	if (txt.length() > 0)
	{
		out = ParseForShownAux(txt);

		// <postmaster@raasu.org> 20030721 -- Strip off trailing line break, we don't need it
		if (out.right(4) == "<br>")
			out.truncate(out.length() - 4);
	}
	return out;
}

QString
ParseForShownAux(const QString &txt)
{
	int n = 0;			// Position of next TAB before text to be included
	int n2 = 0;			// Start of actual text
	int m = 0;			// Position of next TAB after text to be included

	// Remove any extra line breaks from the start of buffer
	//

	// Check for too big text
	if (txt.length() > MAX_BUFFER_SIZE)
	{
		int n3;						// Position of next line break after buffer truncation
		n2 = txt.length() - MAX_BUFFER_SIZE;
		
		// Find next line break

		n3 = txt.find("<br>", n2);
		if (n3 < 0)
		{
			n3 = txt.find('\t', n2);
			if (n3 >= 0)
			{
				n2 = n3 + 1;
			}
		}
		else
		{
			n2 = n3 + 4;
		}
	}

	// Remove any extra line breaks from the start of buffer
	//

	while (n2 < (int) txt.length())
	{
		if (txt.mid(n2, 4) == "<br>")
		{
			n2 += 4;
		}
		else if (txt.mid(n2, 1) == "\t")
		{
			n2++;
		}
		else
		{
			break; // Found start of actual text
		}
	}

	n = txt.find('\t', n2);

	if (n > n2)
	{
		QString out((QChar *) NULL, MAX_BUFFER_SIZE);	// Text after processing linefeeds and TABs, preallocate

		// copy everything before first TAB (after any extra line breaks stripped from the beginning)
		out = txt.mid(n2, n - n2);

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
	else
	{
		return txt.mid(n2);
	}
	return txt;
}

