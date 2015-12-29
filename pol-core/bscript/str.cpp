/*
History
=======
2007/12/09 Shinigami: removed ( is.peek() != EOF ) check from String::unpackWithLen()
                      will not work with Strings in Arrays, Dicts, etc.
2008/02/08 Turley:    String::unpackWithLen() will accept zero length Strings
2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no sense.
2015/20/12 Bodom:     moved most of the methods/props to BaseString

Notes
=======

*/

#include "impstr.h"

#include "berror.h"
#include "bobject.h"
#include "objmethods.h"
#include "executor.h"
#include "../clib/stlutil.h"

#include <cstdlib>
#include <cstring>

#ifdef __GNUG__
#	include <streambuf>
#endif

namespace Pol {
  namespace Bscript {

    /**
     * Creates a String from a char* pointer of a fixed length
     *
     * @param s: the pointer to start reading chars from
     * @param len: Number of chars to read
     */
    String::String( const char *s, int len )
    {
      value_.append( s, len );
    }

    std::string String::pack() const
	{
	  return "s" + value_;
	}

    void String::packonto(std::ostream& os) const
	{
	  os << "S" << value_.size() << ":" << value_;
	}
    void String::packonto(std::ostream& os, const std::string& value)
	{
	  os << "S" << value.size() << ":" << value;
	}

	BObjectImp* String::unpack( const char* pstr )
	{
	  return new String( pstr );
	}

    BObjectImp* String::unpack(std::istream& is)
	{
      std::string tmp;
	  getline( is, tmp );

	  return new String( tmp );
	}

    BObjectImp* String::unpackWithLen(std::istream& is)
	{
	  unsigned len;
	  char colon;
	  if ( !( is >> len >> colon ) )
	  {
		return new BError( "Unable to unpack string length." );
	  }
	  if ( (int)len < 0 )
	  {
		return new BError( "Unable to unpack string length. Invalid length!" );
	  }
	  if ( colon != ':' )
	  {
		return new BError( "Unable to unpack string length. Bad format. Colon not found!" );
	  }

      is.unsetf(std::ios::skipws);
      std::string tmp;
	  tmp.reserve( len );
	  while ( len-- )
	  {
		char ch = '\0';
		if ( !( is >> ch ) || ch == '\0' )
		{
		  return new BError( "Unable to unpack string length. String length excessive." );
		}
		tmp += ch;
	  }

      is.setf(std::ios::skipws);
	  return new String( tmp );
	}

	size_t String::sizeEstimate() const
	{
	  return sizeof(String)+value_.capacity();
	}

	void String::toUpper( void )
	{
	  for( char &c : value_ )
	  {
		c = static_cast<char>(toupper( c ));
	  }
	}

	void String::toLower( void )
	{
	  for( char &c : value_ )
	  {
		c = static_cast<char>(tolower( c ));
	  }
	}


  }
}
