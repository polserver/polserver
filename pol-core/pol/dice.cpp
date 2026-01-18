/** @file
 *
 * @par History
 * - 2005/07/25 Shinigami: added min_value and max_value to populate Weapon-Descriptor
 */


#include "dice.h"

#include <cstring>
#include <ctype.h>
#include <exception>
#include <stdlib.h>

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/random.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"


namespace Pol::Core
{
Dice::Dice() : die_count( 0 ), die_type( 0 ), plus_damage( 0 ) {}

unsigned short Dice::roll() const
{
  return Clib::clamp_convert<unsigned short>( roll_with_negatives() );
}

int Dice::roll_with_negatives() const
{
  int total = 0;
  for ( unsigned i = 0; i < die_count; i++ )
  {
    // random_int produces [0,die_type]
    total += Clib::random_int( die_type - 1 ) + 1;
  }
  total += plus_damage;

  return total;
}

bool Dice::load( const char* dicestr, std::string* errormsg )
{
  try
  {
    const char* str = dicestr;
    char* endptr;


    if ( strchr( str, 'D' ) || strchr( str, 'd' ) )
    {  // xdy or xdy+n or dy + nform
      if ( toupper( *str ) == 'D' )
      {
        str += 1;
        die_count = 1;
      }
      else
      {
        if ( !isdigit( *str ) )
        {
          *errormsg = "Error reading die string '";
          *errormsg += dicestr;
          *errormsg += "': ";
          *errormsg += "Expected digit at beginning";
          return false;
        }

        die_count = strtoul( str, &endptr, 0 );
        if ( die_count == 0 )
        {
          *errormsg = "Error reading die string '";
          *errormsg += dicestr;
          *errormsg += "': ";
          *errormsg += "Got a die count of 0?  strtoul failed?";
          return false;
        }
        // str should point at the 'D'
        if ( toupper( *endptr ) != 'D' )
        {
          *errormsg = "Error reading die string '";
          *errormsg += dicestr;
          *errormsg += "': ";
          *errormsg += "Didn't expect '";
          *errormsg += *endptr;
          *errormsg += "' before the 'D'";
          return false;
        }
        str = endptr + 1;
      }
      // now, we point just past the 'D'. Get the die type.
      if ( !isdigit( *str ) )
      {
        *errormsg = "Error reading die string '";
        *errormsg += dicestr;
        *errormsg += "': ";
        *errormsg += "expected a number after the 'D', got '";
        *errormsg += *str;
        *errormsg += "'";
        return false;
      }
      die_type = strtoul( str, &endptr, 0 );
      if ( !die_type )
      {
        *errormsg = "Error reading die string '";
        *errormsg += dicestr;
        *errormsg += "': ";
        *errormsg += "Die type of '";
        *errormsg += str;
        *errormsg += "' doesn't make sense!";
        return false;
      }

      str = endptr;
      while ( *str && isspace( *str ) )
        str++;
      if ( *str == '\0' )
      {
        plus_damage = 0;
      }
      else
      {
        // Now, we point at the + or - part.
        char sign = *str;
        if ( sign != '+' && sign != '-' )
        {
          *errormsg = "Error reading die string '";
          *errormsg += dicestr;
          *errormsg += "': ";
          *errormsg += "Expected '+' or '-' after xDy, got '";
          *errormsg += sign;
          *errormsg += "'";
          return false;
        }
        str += 1;
        plus_damage = strtoul( str, nullptr, 0 );
        if ( sign == '-' )
          plus_damage = -plus_damage;
      }
    }
    else  // just a number
    {
      if ( !isdigit( *str ) )
      {
        *errormsg = "Error reading die string '";
        *errormsg += dicestr;
        *errormsg += "': ";
        *errormsg += "Expected digit at beginning, got '";
        *errormsg += *str;
        *errormsg += "'";
        return false;
      }
      die_count = 0;
      die_type = 0;
      plus_damage = strtoul( str, nullptr, 0 );
    }
    return true;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN(
        "Uh, looks like I found damage of {} confusing.\n"
        "Valid formats look like: 3D6+2, 2D8, D10, 4, 2d20-4\n"
        "No spaces please!",
        dicestr );
    POLLOGLN( "Dice String {} hurt me!: {}", dicestr, ex.what() );
    *errormsg = "An exception occured trying to decipher dice '";
    *errormsg += dicestr;
    *errormsg += "'";
    return false;
  }
}

void Dice::die_string( std::string& str ) const
{
  OSTRINGSTREAM os;
  os << die_count << "d" << die_type << "+" << plus_damage;
  str = OSTRINGSTREAM_STR( os );
}

unsigned short Dice::min_value() const
{
  int total = die_count + plus_damage;

  if ( total < 0 )
    return 0;
  return static_cast<u16>( total );
}

unsigned short Dice::max_value() const
{
  int total = ( die_count * die_type ) + plus_damage;

  if ( total < 0 )
    return 0;
  return static_cast<u16>( total );
}
}  // namespace Pol::Core
