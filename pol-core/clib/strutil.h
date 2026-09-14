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

#include "clib/rawtypes.h"
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/std.h>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>


namespace Pol::Clib
{
template <typename T>
std::string hexint( T integer )
  requires std::is_integral<T>::value
{
  using namespace fmt::literals;
  return fmt::format( "{:#x}"_cf, integer );
}
template <typename T>
std::string hexint( T integer )
  requires std::is_enum<T>::value
{
  using namespace fmt::literals;
  return fmt::format( "{:#x}"_cf, fmt::underlying( integer ) );
}
template <typename T>
std::string tostring( const T& value )
  requires( !std::is_enum<T>::value )
{
  return fmt::to_string( value );
}
template <typename T>
std::string tostring( const T& value )
  requires std::is_enum<T>::value
{
  return fmt::to_string( fmt::underlying( value ) );
}

/// Writes into the caller's strings, so a line-by-line reader can reuse the same two forever.
void splitnamevalue( std::string_view istr, std::string& propname, std::string& propvalue );
void splitnamevalue( const std::string& istr, std::string& propname, std::string& propvalue );
/// The view from its first character that is not whitespace, empty if it has none. Inline
/// because the config reader asks it of every line of every data file, and nothing links with
/// interprocedural optimization.
inline std::string_view ltrim_view( std::string_view istr )
{
  const auto start = istr.find_first_not_of( " \t\r\n" );
  return start == std::string_view::npos ? std::string_view{} : istr.substr( start );
}
/// splitnamevalue() for a view ltrim_view() has already been applied to, so that a caller which
/// had to find the first character anyway does not pay for finding it twice.
void splitnamevalue_trimmed( std::string_view istr, std::string& propname, std::string& propvalue );

void decodequotedstring( std::string& str );
void encodequotedstring( std::string& str );
std::string getencodedquotedstring( const std::string& in );

// ASCII versions perform a locale dependent conversion
// they are cheaper then the unicode (locale independent) versions
void mklowerASCII( std::string& str );
void mkupperASCII( std::string& str );
std::string strlowerASCII( const std::string& str );
std::string strupperASCII( const std::string& str );

std::string strtrim( const std::string& str );

void remove_bom( std::string* strbuf );
/// The same, without touching the input: returns the view past a leading BOM.
std::string_view remove_bom( std::string_view strbuf );

/// True when no byte has its high bit set. Cheaper than the utf8 validation below, which decodes
/// code point by code point, so it is worth asking first.
bool isPlainAscii( std::string_view str );
bool isValidUnicode( const std::string& str );
// if invalid unicode is detected iso8859 is assumed
void sanitizeUnicodeWithIso( std::string* str );
/// The same, without touching the input: returns str itself when there was nothing to do, and
/// otherwise the rewrite, which is left in *scratch.
std::string_view sanitizeUnicodeWithIso( std::string_view str, std::string* scratch );
// if invalid unicode is detected characters get replaced
void sanitizeUnicode( std::string* str );

// Converts a unicode code point (which is the same as a UTF-32 encoded character, but not the same
// as a UTF-8 or UTF-16 encoded character) into a cp1252 code point (which is the same as a cp1252
// encoded character)
uint8_t unicodeToCp1252( uint32_t codepoint );

// Converts a cp1252 code point into a unicode code point
uint32_t cp1252ToUnicode( uint8_t codepoint );

// Converts a UTF-8 encoded string to CP-1252
std::string strUtf8ToCp1252( const std::string& utf8string );

// Takes a CP-1252 encoded string and returns the same string encoded in UTF-8
std::string strCp1252ToUtf8( const std::string& cp1252string );

bool caseInsensitiveEqual( const std::string& input, const std::string& test );

std::wstring to_wstring( const std::string& value );
}  // namespace Pol::Clib

#endif  // CLIB_STRUTIL_H
