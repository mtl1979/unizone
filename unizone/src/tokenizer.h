#include <qstring.h>

/** String tokenizer class, similar to Java's java.util.StringTokenizer */
class QStringTokenizer
{
public:
   /** Initializes the StringTokenizer to parse (tokenizeMe), which 
    *  should be a string of tokens (e.g. words) separated by any 
    *  of the characters specified in (separators)
    *  @param tokenizeMe the string to break up into 'words'.
    *  @param separators ASCII string representing a list of characters to interpret a word separators.
    *                    Defaults to ", \t" (where \t is of course the tab character)
    */
   QStringTokenizer(const QString &tokenizeMe, const QString &separators = ", \t")
   {
      _seps = separators;
      _next = 0;   
	  _tokenizeMe = tokenizeMe;
   }

   /** Returns the next token in the parsed string, or NULL if there are no more tokens left */
   QString GetNextToken()
   {
      if (_seps.length()>0)
      {
         // Move until first non-sep char
         while	(
				(_next < _tokenizeMe.length()) &&
			    (_seps.find(_tokenizeMe.at(_next)) >= 0)
				) 
				_next++;
         if (_next<_tokenizeMe.length())
         {
            QString ret = _tokenizeMe.mid(_next);
			int prev = _next;
            // Move until next sep-char
            while((_next<_tokenizeMe.length())&&(_seps.find(_tokenizeMe.at(_next)) < 0)) _next++;
            if (_next<_tokenizeMe.length()) 
            {
               ret = _tokenizeMe.mid(prev,_next-prev);
               _next++;
            }
            return ret;
         }
      }
      return QString::null;
   }

   /** Convenience synonym for GetNextToken() */
   QString operator()() {return GetNextToken();}
 
   /** Returns the remainder of the string, starting with the next token,
    *  or NULL if there are no more tokens in the string.
    *  Doesn't affect the next return value of GetNextToken(), though.
    */
   QString GetRemainderOfString()
   {
      if (_seps.length()>0)
      {
         // Move until first non-sep char
         while((_next<_tokenizeMe.length())&&(_seps.find(_tokenizeMe.at(_next)) >= 0)) _next++;
         return (_next<_tokenizeMe.length()) ? _tokenizeMe.mid(_next) : QString::null;  // and return from there
      }
      return QString::null;
   }

private:
   QStringTokenizer(const QStringTokenizer &);   // unimplemented on purpose
   QStringTokenizer & operator = (const QStringTokenizer &);  // unimplemented on purpose

   QString _seps, _tokenizeMe;
   uint _next;
};
