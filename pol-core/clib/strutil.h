/** @file
 *
 * @par History
 *
 * @note ATTENTION:
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#ifndef CLIB_STRUTIL_H
#define CLIB_STRUTIL_H

#include "rawtypes.h"
#include <format/format.h>
#include <string>
#include <type_traits>

namespace Pol
{
namespace Clib
{
template <typename T>
typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value, std::string>::type
hexint( T integer )
{
  return fmt::str( fmt::Writer() << "0x" << fmt::hex( integer ) );
}

template <typename T>
std::string tostring( const T& value )
{
  return fmt::str( fmt::Writer() << value );
}

void splitnamevalue( const std::string& istr, std::string& propname, std::string& propvalue );

void decodequotedstring( std::string& str );
void encodequotedstring( std::string& str );
std::string getencodedquotedstring( const std::string& in );

// ASCII versions perform a locale dependent conversion
// they are cheaper then the unicode (locale independent) versions
void mklowerASCII( std::string& str );
void mkupperASCII( std::string& str );
std::string strlowerASCII( const std::string& str );
std::string strupperASCII( const std::string& str );

void remove_bom( std::string* strbuf );

bool isValidUnicode( const std::string& str );
// if invalid unicode is detected iso8859 is assumed
void sanitizeUnicodeWithIso( std::string* str );
// if invalid unicode is detected characters get replaced
void sanitizeUnicode( std::string* str );
std::string convertToASCII( const std::string& str);
}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_STRUTIL_H
