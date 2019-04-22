/** @file
 *
 * @par History
 * - 2005/09/10 Shinigami: added mf_Root - calculates y Root of x as a Real
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 *                         removed obsolete gcvt, used sprintf
 */


#include "mathmod.h"
#include "modules/math-tbl.h"
#include <cmath>
#include <stdio.h>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"


namespace Pol
{
namespace Module
{
using namespace Bscript;

static double const_pi;
static double const_e;

class initer
{
public:
  initer();
};
initer::initer()
{
  const_pi = acos( double( -1 ) );
  const_e = exp( double( 1 ) );
}
initer _initer;

MathExecutorModule::MathExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<MathExecutorModule>( exec )
{
}

Bscript::BObjectImp* MathExecutorModule::mf_Sin()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( sin( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_Cos()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( cos( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_Tan()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( tan( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_ASin()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( asin( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_ACos()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( acos( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_ATan()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( atan( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

/*
math::Pow(x : number, y : number ) : real
Function: calculates x raised to the power of y
Returns:  x^y as a Real
Parameters: x and y can be Reals or Integers

math::Pow(2,5) = 2^5 = 32
*/
Bscript::BObjectImp* MathExecutorModule::mf_Pow()
{
  double x, y;
  if ( getRealParam( 0, x ) && getRealParam( 1, y ) )
  {
    return new Double( pow( x, y ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

/*
math::Sqrt( x : number ) : real
Returns: Square Root of x as a Real
*/
Bscript::BObjectImp* MathExecutorModule::mf_Sqrt()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( sqrt( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

/*
math::Root( x : number, y : number ) : real
Function: calculates y Root of x as a Real
*/
Bscript::BObjectImp* MathExecutorModule::mf_Root()
{
  double x, y;
  if ( getRealParam( 0, x ) && getRealParam( 1, y ) )
  {
    return new Bscript::Double( pow( x, 1.0 / y ) );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* MathExecutorModule::mf_Min()
{
  Bscript::BObjectImp* impX = getParamImp( 0 );
  Bscript::BObjectImp* impY = getParamImp( 1 );
  if ( ( ( impX->isa( Bscript::BObjectImp::OTDouble ) ) ||
         ( impX->isa( Bscript::BObjectImp::OTLong ) ) ) &&
       ( ( impY->isa( Bscript::BObjectImp::OTDouble ) ) ||
         ( impY->isa( Bscript::BObjectImp::OTLong ) ) ) )
  {
    if ( *impX < *impY )
      return impX->copy();
    else
      return impY->copy();
  }
  else if ( impX->isa( Bscript::BObjectImp::OTArray ) )
  {
    Bscript::ObjArray* value = static_cast<Bscript::ObjArray*>( impX );
    if ( value->ref_arr.empty() )
      return new Bscript::BError( "Array empty" );

    Bscript::BObjectImp* compare = nullptr;
    for ( std::vector<Bscript::BObjectRef>::iterator itr = value->ref_arr.begin();
          itr != value->ref_arr.end(); ++itr )
    {
      if ( itr->get() )
      {
        Bscript::BObject* bo = ( itr->get() );

        if ( bo == nullptr )
          continue;
        if ( ( bo->isa( Bscript::BObjectImp::OTDouble ) ) ||
             ( bo->isa( Bscript::BObjectImp::OTLong ) ) )
        {
          if ( compare == nullptr )
            compare = bo->impptr();
          else if ( *( bo->impptr() ) < *compare )
            compare = bo->impptr();
        }
      }
    }
    if ( compare != nullptr )
      return ( compare->copy() );
    else
      return new Bscript::BError( "No Integer/Double elements" );
  }
  else
    return new Bscript::BError( "Invalid parameter type" );
}

Bscript::BObjectImp* MathExecutorModule::mf_Max()
{
  Bscript::BObjectImp* impX = getParamImp( 0 );
  Bscript::BObjectImp* impY = getParamImp( 1 );
  if ( ( ( impX->isa( Bscript::BObjectImp::OTDouble ) ) ||
         ( impX->isa( Bscript::BObjectImp::OTLong ) ) ) &&
       ( ( impY->isa( Bscript::BObjectImp::OTDouble ) ) ||
         ( impY->isa( Bscript::BObjectImp::OTLong ) ) ) )
  {
    if ( *impX < *impY )
      return impY->copy();
    else
      return impX->copy();
  }
  else if ( impX->isa( Bscript::BObjectImp::OTArray ) )
  {
    Bscript::ObjArray* value = static_cast<Bscript::ObjArray*>( impX );
    if ( value->ref_arr.empty() )
      return new Bscript::BError( "Array empty" );

    Bscript::BObjectImp* compare = nullptr;
    for ( std::vector<BObjectRef>::iterator itr = value->ref_arr.begin();
          itr != value->ref_arr.end(); ++itr )
    {
      if ( itr->get() )
      {
        Bscript::BObject* bo = ( itr->get() );

        if ( bo == nullptr )
          continue;
        if ( ( bo->isa( Bscript::BObjectImp::OTDouble ) ) ||
             ( bo->isa( Bscript::BObjectImp::OTLong ) ) )
        {
          if ( compare == nullptr )
            compare = bo->impptr();
          else if ( *( bo->impptr() ) >= *compare )
            compare = bo->impptr();
        }
      }
    }
    if ( compare != nullptr )
      return ( compare->copy() );
    else
      return new Bscript::BError( "No Integer/Double elements" );
  }
  else
    return new Bscript::BError( "Invalid parameter type" );
}

/*
math::Abs( x : number) : number
Returns: the absolute value of x
If a Real is passed, will return a Real
If an Integer is passed, will return an Integer

math::Abs(-2) = 2
math::Abs(-1.5) = 1.5
*/
Bscript::BObjectImp* MathExecutorModule::mf_Abs()
{
  Bscript::BObjectImp* imp = getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
  {
    double value = static_cast<Double*>( imp )->value();
    return new Double( fabs( value ) );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    int value = static_cast<BLong*>( imp )->value();
    return new BLong( labs( value ) );
  }
  else
  {
    double x;
    // just for debug.log
    (void)getRealParam( 0, x );
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* MathExecutorModule::mf_Log10()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( log10( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_LogE()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( log( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* MathExecutorModule::mf_ConstPi()
{
  return new Double( const_pi );
}
Bscript::BObjectImp* MathExecutorModule::mf_ConstE()
{
  return new Double( const_e );
}

Bscript::BObjectImp* MathExecutorModule::mf_FormatRealToString()
{
  double x;
  int digits;
  if ( getRealParam( 0, x ) && getParam( 1, digits ) )
  {
    char buffer[200];
    /// @todo: sprintf produces different output on some doubles (eg. M_PI)
    ///        on Windows/Linux. Use something else? 2016-01-23 Bodom
    /// Turley: as of now 2018-10-30 vs2017 on appveyor and travis show no difference with 64bit. Is
    /// it just a difference between 64bit and 32bit?
    sprintf( buffer, "%.*g", static_cast<int>( digits ), x );
    return new String( buffer );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* MathExecutorModule::mf_RadToDeg()
{
  double rad;
  if ( !getRealParam( 0, rad ) )
    return new BError( "Invalid parameter type" );

  /*
      360 degrees <~> 2*pi radians
      <=> deg/360 = rad / 2*pi
      <=> deg = 360*rad / 2*pi
      <=> deg = 180*rad / pi
      */

  return new Double( ( rad * 180.0 ) / const_pi );
}

Bscript::BObjectImp* MathExecutorModule::mf_DegToRad()
{
  double deg;
  if ( !getRealParam( 0, deg ) )
    return new BError( "Invalid parameter type" );

  /*
      2*pi radians <~> 360 degrees
      <=> rad / 2*pi = deg / 360
      <=> rad = 2*pi*deg / 360
      <=> rad = deg * pi / 180
      */
  return new Double( ( deg * const_pi ) / 180.0 );
}

Bscript::BObjectImp* MathExecutorModule::mf_Ceil()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( ceil( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* MathExecutorModule::mf_Floor()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( floor( x ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
}  // namespace Module
}  // namespace Pol
