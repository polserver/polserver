/** @file
 *
 * @par History
 * - 2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no
 * sense.
 * - 2014/06/10 Nando:  Removed pragma that disabled 4244. (tolower()/toupper() used ints because -1
 * is a valid output).
 */


#include "strutil.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
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

std::string strtrim( const std::string& str )
{
  return boost::trim_copy( str );
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

uint8_t unicodeToCp1252( uint32_t codepoint )
{
  if ( codepoint >= 0x80 && codepoint <= 0x9f )
    return '?';
  else if ( codepoint <= 0xff )
    return (char)codepoint;
  else
  {
    switch ( codepoint )
    {
      case 0x20AC: return 128; break; //€
      case 0x201A: return 130; break; //‚
      case 0x0192: return 131; break; //ƒ
      case 0x201E: return 132; break; //„
      case 0x2026: return 133; break; //…
      case 0x2020: return 134; break; //†
      case 0x2021: return 135; break; //‡
      case 0x02C6: return 136; break; //ˆ
      case 0x2030: return 137; break; //‰
      case 0x0160: return 138; break; //Š
      case 0x2039: return 139; break; //‹
      case 0x0152: return 140; break; //Œ
      case 0x017D: return 142; break; //Ž
      case 0x2018: return 145; break; //‘
      case 0x2019: return 146; break; //’
      case 0x201C: return 147; break; //“
      case 0x201D: return 148; break; //”
      case 0x2022: return 149; break; //•
      case 0x2013: return 150; break; //–
      case 0x2014: return 151; break; //—
      case 0x02DC: return 152; break; //˜
      case 0x2122: return 153; break; //™
      case 0x0161: return 154; break; //š
      case 0x203A: return 155; break; //›
      case 0x0153: return 156; break; //œ
      case 0x017E: return 158; break; //ž
      case 0x0178: return 159; break; //Ÿ
      default: return '?'; break;
    }
  }
}

uint32_t cp1252ToUnicode( uint8_t codepoint )
{
  switch ( codepoint )
  {
    case 128: return 0x20AC; break; //€
    case 130: return 0x201A; break; //‚
    case 131: return 0x0192; break; //ƒ
    case 132: return 0x201E; break; //„
    case 133: return 0x2026; break; //…
    case 134: return 0x2020; break; //†
    case 135: return 0x2021; break; //‡
    case 136: return 0x02C6; break; //ˆ
    case 137: return 0x2030; break; //‰
    case 138: return 0x0160; break; //Š
    case 139: return 0x2039; break; //‹
    case 140: return 0x0152; break; //Œ
    case 142: return 0x017D; break; //Ž
    case 145: return 0x2018; break; //‘
    case 146: return 0x2019; break; //’
    case 147: return 0x201C; break; //“
    case 148: return 0x201D; break; //”
    case 149: return 0x2022; break; //•
    case 150: return 0x2013; break; //–
    case 151: return 0x2014; break; //—
    case 152: return 0x02DC; break; //˜
    case 153: return 0x2122; break; //™
    case 154: return 0x0161; break; //š
    case 155: return 0x203A; break; //›
    case 156: return 0x0153; break; //œ
    case 158: return 0x017E; break; //ž
    case 159: return 0x0178; break; //Ÿ
    default: return codepoint; break;
  }
}

std::string strUtf8ToCp1252( const std::string &utf8string )
{
  auto itr = utf8string.begin();
  auto end = utf8string.end();
  std::string outstring;
  while( itr != end )
  {
    auto c = unicodeToCp1252( utf8::unchecked::next( itr ) );
    outstring.push_back( c );
  }
  return outstring;
}

std::string strCp1252ToUtf8( const std::string &cp1252string )
{
  auto itr = cp1252string.begin();
  auto end = cp1252string.end();
  std::string outstring;
  auto inserter = std::back_inserter(outstring);
  while( itr != end )
  {
    utf8::unchecked::append(cp1252ToUnicode(*itr++), inserter);
  }
  return outstring;
}

bool caseInsensitiveEqual( const std::string& input, const std::string& test )
{
  return boost::iequals( input, test );
}


}  // namespace Clib
}  // namespace Pol
