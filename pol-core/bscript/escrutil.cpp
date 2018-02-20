/** @file
 *
 * @par History
 * - 2005/07/06 Shinigami: convert_numeric will use string representation if overflow
 */


#include "escrutil.h"

#include <climits>
#include <cmath>
#include <ctype.h>
#include <stdlib.h>

#include "bobject.h"
#include "impstr.h"

namespace Pol
{
namespace Bscript
{
bool could_be_a_number( const char* s )
{
  if ( s[0] == '0' && ( s[1] == 'x' || s[1] == 'X' ) )  // Hex number
  {
    s += 2;
    while ( *s )
    {
      char ch = *s;
      ++s;
      if ( isxdigit( ch ) )
        continue;
      else if ( isspace( ch ) )
        return true;
      else
        return false;
    }
    return true;
  }
  else  // expect -, +, 0-9, . only
  {
    while ( *s )
    {
      char ch = *s;
      ++s;
      if ( ch == '-' || ch == '+' || ( ch >= '0' && ch <= '9' ) || ch == '.' )
      {
        continue;
      }
      else
      {
        return false;
      }
    }
    return true;
  }
}

BObjectImp* convert_numeric( const std::string& str, int radix )
{
  const char* s = str.c_str();
  int ch = static_cast<unsigned char>( s[0] );
  if ( isdigit( ch ) || ch == '.' || ch == '+' || ch == '-' )
  {
    char *endptr = NULL, *endptr2 = NULL;
    int l = strtol( s, &endptr, radix );
    double d = strtod( s, &endptr2 );

    if ( endptr >= endptr2 )
    {
      // it's a long
      if ( endptr )
      {
        if ( ( l != INT_MIN ) && ( l != INT_MAX ) )
        {
          while ( *endptr )
          {
            if ( !isspace( *endptr ) )
            {
              if ( *endptr == '/' && *( endptr + 1 ) == '/' )
              {  // what follows is a comment
                break;
              }
              return NULL;
            }
            ++endptr;
          }
        }
        else
          return NULL;  // overflow, read it as string
      }
      return new BLong( l );
    }
    else
    {
      if ( !could_be_a_number( s ) )
        return NULL;
      if ( endptr2 )
      {
        if ( ( d != -HUGE_VAL ) && ( d != +HUGE_VAL ) )
        {
          while ( *endptr2 )
          {
            if ( !isspace( *endptr2 ) )
            {
              if ( *endptr2 == '/' && *( endptr2 + 1 ) == '/' )
              {
                // assume what follows is a comment
                break;
              }
              return NULL;
            }
            ++endptr2;
          }
        }
        else
          return NULL;  // overflow, read it as string
      }
      return new Double( d );
    }
  }
  return NULL;
}

BObjectImp* bobject_from_string( const std::string& str, int radix )
{
  BObjectImp* imp = convert_numeric( str, radix );
  if ( imp )
    return imp;
  else
    return new ConstString( str );
}


std::string normalize_ecl_filename( const std::string& filename )
{
  if ( filename.find( ".ecl" ) == std::string::npos )
    return filename + ".ecl";
  else
    return filename;
}
}
}
