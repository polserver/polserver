/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2008/07/08 Turley:    Added mf_RandomIntMinMax - Return Random Value between...
 * - 2011/01/07 Nando:     fix uninit in mf_StrFormatTime - strftime's return is now tested
 */


#include "utilmod.h"
#include <algorithm>
#include <ctime>
#include <string>
#include <vector>

#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib.h"
#include "../../clib/random.h"
#include "../dice.h"

#include <module_defs/util.h>

namespace Pol
{
namespace Module
{
using namespace Bscript;

UtilExecutorModule::UtilExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<UtilExecutorModule, Bscript::ExecutorModule>( exec )
{
}

Bscript::BObjectImp* UtilExecutorModule::mf_RandomInt()
{
  int value;
  if ( exec.getParam( 0, value, 1, INT_MAX ) )
  {
    if ( value > 0 )
      return new BLong( Clib::random_int( value - 1 ) );
    else
      return new BError( "RandomInt() expects a positive integer" );
  }
  else
  {
    return new BError( "RandomInt() expects a positive integer" );
  }
}

Bscript::BObjectImp* UtilExecutorModule::mf_RandomIntMinMax()
{
  int minvalue;
  if ( exec.getParam( 0, minvalue, INT_MIN, INT_MAX ) )
  {
    int maxvalue;
    if ( exec.getParam( 1, maxvalue, INT_MIN, INT_MAX ) )
    {
      maxvalue--;
      return new BLong( Clib::random_int_range( minvalue, maxvalue ) );
    }
    else
      return new BError( "RandomIntMinMax() expects an integer" );
  }
  else
    return new BError( "RandomIntMinMax() expects an integer" );
}

Bscript::BObjectImp* UtilExecutorModule::mf_RandomFloat()
{
  double value;
  if ( exec.getRealParam( 0, value ) )
  {
    return new Double( Clib::random_double( value ) );
  }
  else
  {
    return new BError( "RandomFloat() expects a Real parameter" );
  }
}

Bscript::BObjectImp* UtilExecutorModule::mf_RandomDiceRoll()
{
  const String* dicestr;
  bool allow_negatives;

  if ( exec.getStringParam( 0, dicestr ) && exec.getParam( 1, allow_negatives ) )
  {
    std::string errormsg;
    Core::Dice dice;
    if ( dice.load( dicestr->data(), &errormsg ) )
    {
      if ( allow_negatives )
        return new BLong( dice.roll_with_negatives() );
      else
        return new BLong( dice.roll() );
    }
    else
    {
      return new BError( errormsg.c_str() );
    }
  }
  else
  {
    return new BError( "RandomDiceRoll() expects a String as parameter" );
  }
}

Bscript::BObjectImp* UtilExecutorModule::mf_StrFormatTime()
{
  const String* format_string;
  if ( !getStringParam( 0, format_string ) )
    return new BError( "No time string passed." );

  int time_stamp;
  if ( !getParam( 1, time_stamp, 0, INT_MAX ) )
    time_stamp = 0;

  time_t seconds;


  if ( time_stamp <= 0 )
    seconds = time( nullptr );
  else
    seconds = time_stamp;

  auto time_struct = Clib::localtime( seconds );

  // strftime uses assert check for invalid format -> precheck it
  size_t len = format_string->length();
  const char* str = format_string->data();
  while ( len-- > 0 )
  {
    if ( *str++ == '%' )
    {
      if ( len-- <= 0 )
        return new BError( "Invalid Format string." );
      switch ( *str++ )
      {
        // vs2019 does not support 0 and E formats
        /*
      case ( '0' ):
      {
        if ( len-- <= 0 )
          return new BError( "Invalid Format string." );
        switch ( *str++ )
        {
        case ( 'd' ):
        case ( 'e' ):
        case ( 'H' ):
        case ( 'I' ):
        case ( 'm' ):
        case ( 'M' ):
        case ( 'S' ):
        case ( 'u' ):
        case ( 'U' ):
        case ( 'V' ):
        case ( 'w' ):
        case ( 'W' ):
        case ( 'y' ):
          continue;
        default:
          return new BError( "Invalid Format string." );
        }
        continue;
      }
      case ( 'E' ):
      {
        if ( len-- <= 0 )
          return new BError( "Invalid Format string." );
        switch ( *str++ )
        {
        case ( 'c' ):
        case ( 'C' ):
        case ( 'x' ):
        case ( 'X' ):
        case ( 'y' ):
        case ( 'Y' ):
          continue;
        default:
          return new BError( "Invalid Format string." );
        }
        continue;
      }
      */
      case ( '%' ):
      case ( 'a' ):
      case ( 'A' ):
      case ( 'b' ):
      case ( 'B' ):
      case ( 'c' ):
      case ( 'C' ):
      case ( 'd' ):
      case ( 'D' ):
      case ( 'e' ):
      case ( 'F' ):
      case ( 'g' ):
      case ( 'G' ):
      case ( 'h' ):
      case ( 'H' ):
      case ( 'I' ):
      case ( 'j' ):
      case ( 'n' ):
      case ( 'm' ):
      case ( 'M' ):
      case ( 'p' ):
      case ( 'r' ):
      case ( 'R' ):
      case ( 'S' ):
      case ( 't' ):
      case ( 'T' ):
      case ( 'u' ):
      case ( 'U' ):
      case ( 'V' ):
      case ( 'w' ):
      case ( 'W' ):
      case ( 'x' ):
      case ( 'X' ):
      case ( 'y' ):
      case ( 'Y' ):
      case ( 'z' ):
      case ( 'Z' ):
        continue;
      case ( '\0' ):
        len = 0;
        break;
      default:
        return new BError( "Invalid Format string." );
      }
    }
  }

  std::vector<char> buffer;
  buffer.resize( std::max( format_string->length() * 2, (size_t)50u ) );
  while ( strftime( buffer.data(), buffer.capacity(), format_string->data(), &time_struct ) == 0 )
  {
    buffer.resize( buffer.capacity() * 2 );
  }
  return new String( buffer.data() );
}
}  // namespace Module
}  // namespace Pol
