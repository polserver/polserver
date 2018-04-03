/*
History
=======
2015/20/12 Bodom:     creating this file by copy/pasting str.cpp (shame on me)

Notes
=======

*/

#include "impunicode.h"

#include "berror.h"
#include "bobject.h"
#include "objmethods.h"
#include "executor.h"
#include "../clib/stlutil.h"

#include <cstdlib>
#include <cstring>

#ifdef __GNUG__
#include <streambuf>
#endif

namespace Pol {
namespace Bscript {

/**
 * Creates a Unicode from a UnicodeChar* pointer of a fixed length
 *
 * @param s: the pointer to start reading chars from
 * @param len: Number of chars to read
 */
Unicode::Unicode( const UnicodeChar *s, int len )
{
  value_.append( s, len );
}

std::string Unicode::pack() const
{
  return "U" + value_.asUtf8();
}

void Unicode::packonto(std::ostream& os) const
{
  os << "U" << value_.asUtf8();
}
void Unicode::packonto(std::ostream& os, const UnicodeString& value)
{
  os << "U" << value_.asUtf8();
}

BObjectImp* Unicode::unpack( const char* pstr )
{
  Utf8CharValidator validator;
  UnicodeString ustr;

  char* end = pstr;
  while( *end )
  {
    switch( validator.addByte(*end) )
    {
    case Pol::Utf8CharValidator::MORE:
      end++;
      if( ! *end )
        return BError( "Error extracting Unicode value: truncated" );
      continue;
    case Pol::Utf8CharValidator::INVALID:
      return BError( "Error extracting Unicode value: invalid" );
    default:
      assert(false); //This should never happen
    case Pol::Utf8CharValidator::DONE:
      break;
    }

    try {
      ustr += validator.getChar().asUtf16();
    } catch( const UnicodeCastFailedException& ) {
      return BError( "Error extracting Unicode value: out fo range" );
    }

    validator.reset();
    end++;
  }

  return ustr;
}

BObjectImp* Unicode::unpack(std::istream& is)
{
  //TODO: should probably improve this
  std::string tmp;
  getline( is, tmp );
  return unpack(tmp.c_str());
}

size_t Unicode::sizeEstimate() const
{
  return sizeof(Unicode) + value_.sizeEstimate();
}

void Unicode::toUpper( void )
{
  value_.toUpper();
}

void Unicode::toLower( void )
{
  value_.toLower();
}

}
}
