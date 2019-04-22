/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2008/07/08 Turley:    Added mf_RandomIntMinMax - Return Random Value between...
 * - 2011/01/07 Nando:     fix uninit in mf_StrFormatTime - strftime's return is now tested
 */


#include "utilmod.h"
#include "modules/util-tbl.h"

#include <ctime>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib.h"
#include "../../clib/random.h"
#include "../dice.h"

namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<UtilExecutorModule>::FunctionTable
    TmplExecutorModule<UtilExecutorModule>::function_table = {
        {"RandomInt", &UtilExecutorModule::mf_RandomInt},
        {"RandomFloat", &UtilExecutorModule::mf_RandomFloat},
        {"RandomDiceRoll", &UtilExecutorModule::mf_RandomDiceRoll},
        {"StrFormatTime", &UtilExecutorModule::mf_StrFormatTime},
        {"RandomIntMinMax", &UtilExecutorModule::mf_RandomIntMinMax}};
}  // namespace Bscript
namespace Module
{
using namespace Bscript;

UtilExecutorModule::UtilExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<UtilExecutorModule>( "util", exec )
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
  if ( exec.getStringParam( 0, dicestr ) )
  {
    std::string errormsg;
    Core::Dice dice;
    if ( dice.load( dicestr->data(), &errormsg ) )
    {
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
  else if ( format_string->length() > 100 )
    return new BError( "Format string exceeded 100 characters." );

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
      case ( '%' ):
      case ( 'a' ):
      case ( 'A' ):
      case ( 'b' ):
      case ( 'B' ):
      case ( 'c' ):
      case ( 'd' ):
      case ( 'H' ):
      case ( 'I' ):
      case ( 'j' ):
      case ( 'm' ):
      case ( 'M' ):
      case ( 'p' ):
      case ( 'S' ):
      case ( 'U' ):
      case ( 'w' ):
      case ( 'W' ):
      case ( 'x' ):
      case ( 'X' ):
      case ( 'y' ):
      case ( 'Y' ):
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

  char buffer[102];  // +2 for the \0 termination.
  if ( strftime( buffer, sizeof buffer, format_string->data(), &time_struct ) > 0 )
    return new String( buffer );
  else
    return new BError( "Format string too long." );
}
}  // namespace Module
}  // namespace Pol
