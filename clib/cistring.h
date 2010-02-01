/*
History
=======

Notes
=======

*/

#ifndef H_CISTRING_H
#define H_CISTRING_H

#include <string>
#include <string.h>

#include "clib.h"

// use ci_cmp
#if 0

#	if 0 && defined(__GNUC__)
		// EGCS 1.1: char_traits<char>
	    // EGCS 1.2: string_char_traits<char>
		struct ci_char_traits : public string_char_traits<char>
#	else
		struct ci_char_traits : public char_traits<char>
#	endif
{
            // just inherit all the other functions
            //  that we don't need to override
    static bool eq( char c1, char c2 ) 
    {
      return tolower(c1) == tolower(c2);
    }

    static bool ne( char c1, char c2 ) 
    {
      return tolower(c1) != tolower(c2);
    }

    static bool lt( char c1, char c2 ) 
    {
      return tolower(c1) < tolower(c2);
    }

    static int compare( const char* s1,
                        const char* s2,
                        size_t n ) 
    {
        return strnicmp( s1, s2, n );
    }

    static const char*
    find( const char* s, int n, char a ) 
    {
      while( n-- > 0 && tolower(*s) != tolower(a) ) 
      {
          ++s;
      }
      return s;
    }
};

typedef basic_string<char, ci_char_traits> ci_string;

#endif

#endif
