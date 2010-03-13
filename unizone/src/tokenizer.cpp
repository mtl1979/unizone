/** String tokenizer class, similar to Java's java.util.StringTokenizer */

#include <QString>

#include "tokenizer.h"

QStringTokenizer::QStringTokenizer(const QString &tokenizeMe, const QString &separators)
{
	_seps = separators;
	_next = 0;
	_tokenizeMe = tokenizeMe;
}

/** Returns the next token in the parsed string, or NULL if there are no more tokens left */
QString 
QStringTokenizer::GetNextToken()
{
	if (!_seps.isEmpty())
	{
		// Move until first non-sep char
		while	(
			(_next < _tokenizeMe.length()) &&
			(_seps.find( _tokenizeMe.at(_next) ) >= 0)
			) 
		{
			_next++;
		}
		if (_next < _tokenizeMe.length())
		{
			QString ret = _tokenizeMe.mid(_next);
			int prev = _next;
			// Move until next sep-char
			while	( 
				(_next < _tokenizeMe.length()) && 
				( _seps.find( _tokenizeMe.at(_next) ) < 0) 
				)
			{
				_next++;
			}

			if (_next < _tokenizeMe.length()) 
			{
				ret = _tokenizeMe.mid(prev,_next-prev);
				_next++;
			}
			return ret;
		}
	}
	return QString::null;
}

/**  Returns the remainder of the string, starting with the next token,
  *  or NULL if there are no more tokens in the string.
  *  Doesn't affect the next return value of GetNextToken(), though.
  */
QString 
QStringTokenizer::GetRemainderOfString()
{
	if (!_seps.isEmpty())
	{
		// Move until first non-sep char
		while((_next<_tokenizeMe.length())&&(_seps.find(_tokenizeMe.at(_next)) >= 0)) _next++;
		return (_next<_tokenizeMe.length()) ? _tokenizeMe.mid(_next) : QString::null;  // and return from there
	}
	return QString::null;
}
