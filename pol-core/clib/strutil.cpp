/** @file
 *
 * @par History
 * - 2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no
 * sense.
 * - 2014/06/10 Nando:  Removed pragma that disabled 4244. (tolower()/toupper() used ints because -1
 * is a valid output).
 */


#include "strutil.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <utf8/utf8.h>

#include "logfacility.h"
#include "stlutil.h"

namespace Pol
{
namespace Clib
{
void splitnamevalue( const std::string& istr, std::string& propname, std::string& propvalue )
{
  std::string::size_type start = istr.find_first_not_of( " \t\r\n" );
  if ( start != std::string::npos )
  {
    std::string::size_type delimpos = istr.find_first_of( " \t\r\n=", start + 1 );
    if ( delimpos != std::string::npos )
    {
      std::string::size_type valuestart = istr.find_first_not_of( " \t\r\n", delimpos + 1 );
      std::string::size_type valueend = istr.find_last_not_of( " \t\r\n" );
      propname = istr.substr( start, delimpos - start );
      if ( valuestart != std::string::npos && valueend != std::string::npos )
      {
        propvalue = istr.substr( valuestart, valueend - valuestart + 1 );
      }
      else
      {
        propvalue = "";
      }
    }
    else
    {
      propname = istr.substr( start, std::string::npos );
      propvalue = "";
    }
  }
  else
  {
    propname = "";
    propvalue = "";
  }
}

void decodequotedstring( std::string& str )
{
  std::string tmp;
  tmp.swap( str );
  const char* s = tmp.c_str();
  str.reserve( tmp.size() );
  ++s;
  while ( *s )
  {
    char ch = *s++;

    switch ( ch )
    {
    case '\\':
      ch = *s++;
      switch ( ch )
      {
      case '\0':
        return;
      case 'n':  // newline
        str += "\n";
        break;
      default:  // slash, quote, etc
        str += ch;
        break;
      }
      break;

    case '\"':
      return;

    default:
      str += ch;
      break;
    }
  }
}
void encodequotedstring( std::string& str )
{
  std::string tmp;
  tmp.swap( str );
  const char* s = tmp.c_str();
  str.reserve( tmp.size() + 2 );
  str += "\"";

  while ( *s )
  {
    char ch = *s++;
    switch ( ch )
    {
    case '\\':
      str += "\\\\";
      break;
    case '\"':
      str += "\\\"";
      break;
    case '\n':
      str += "\\n";
      break;
    default:
      str += ch;
      break;
    }
  }

  str += "\"";
}

std::string getencodedquotedstring( const std::string& in )
{
  std::string tmp = in;
  encodequotedstring( tmp );
  return tmp;
}

// If we have boost, I think we should use it...
void mklowerASCII( std::string& str )
{
  boost::to_lower( str );
}

void mkupperASCII( std::string& str )
{
  boost::to_upper( str );
}

std::string strlowerASCII( const std::string& str )
{
  return boost::to_lower_copy( str );
}

std::string strupperASCII( const std::string& str )
{
  return boost::to_upper_copy( str );
}

bool isValidUnicode( const std::string& str )
{
  return utf8::find_invalid( str.begin(), str.end() ) == str.end();
}

void sanitizeUnicodeWithIso( std::string* str )
{
  if ( isValidUnicode( *str ) )
    return;
  // assume iso8859
  std::string utf8( "" );
  utf8.reserve( 2 * str->size() + 1 );

  for ( const auto& s : *str )
  {
    if ( !( s & 0x80 ) )
    {
      utf8.push_back( s );
    }
    else
    {
      utf8.push_back( 0xc2 | ( (unsigned char)( s ) >> 6 ) );
      utf8.push_back( 0xbf & s );
    }
  }
  *str = utf8;
}

void sanitizeUnicode( std::string* str )
{
  if ( !isValidUnicode( *str ) )
  {
    try
    {
      std::string new_s;
      utf8::replace_invalid( str->begin(), str->end(), std::back_inserter( new_s ) );
      *str = new_s;
    }
    catch ( utf8::exception& )
    {
      *str = "Invalid unicode";
    }
  }
  auto begin = str->begin();
  auto end = str->end();
  auto invalid_chr = []( u32 c )
  {
    return ( c >= 0x1u && c < 0x9u ) /*0x9 \t 0x10 \n*/ ||
           ( c >= 0x11u && c < 0x13u ) /*0x13 \r*/ || ( c >= 0x14u && c < 0x20u ) || c == 0x7Fu ||
           ( c >= 0x80u && c <= 0x9Fu );
  };
  while ( begin != end )
  {
    auto c = utf8::unchecked::next( begin );
    if ( invalid_chr( c ) )
    {
      // control character found build new string skipping them
      std::string new_s;
      begin = str->begin();
      while ( begin != end )
      {
        c = utf8::unchecked::next( begin );
        if ( invalid_chr( c ) )
          continue;
        utf8::unchecked::append( c, std::back_inserter( new_s ) );
      }
      *str = new_s;
      break;
    }
  }
}

void remove_bom( std::string* strbuf )
{
  if ( strbuf->size() >= 3 )
  {
    if ( utf8::starts_with_bom( strbuf->cbegin(), strbuf->cend() ) )
      strbuf->erase( 0, 3 );
  }
}

}  // namespace Clib
}  // namespace Pol
