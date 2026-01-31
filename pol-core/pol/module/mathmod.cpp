/** @file
 *
 * @par History
 * - 2005/09/10 Shinigami: added mf_Root - calculates y Root of x as a Real
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 *                         removed obsolete gcvt, used sprintf
 */


#include "mathmod.h"
#include <cmath>
#include <stdio.h>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../clib/stlutil.h"

#include <module_defs/math.h>


namespace Pol::Module
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
    : Bscript::TmplExecutorModule<MathExecutorModule, Bscript::ExecutorModule>( exec )
{
}

Bscript::BObjectImp* MathExecutorModule::mf_Sin()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( sin( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_Cos()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( cos( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_Tan()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( tan( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_ASin()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( asin( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_ACos()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( acos( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_ATan()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( atan( x ) );
  }

  return new BError( "Invalid parameter type" );
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

  return new BError( "Invalid parameter type" );
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

  return new BError( "Invalid parameter type" );
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
    return new Double( pow( x, 1.0 / y ) );
  }

  return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* MathExecutorModule::mf_Min()
{
  BObjectImp* impX = getParamImp( 0 );
  BObjectImp* impY = getParamImp( 1 );
  if ( ( ( impX->isa( BObjectImp::OTDouble ) ) || ( impX->isa( BObjectImp::OTLong ) ) ) &&
       ( ( impY->isa( BObjectImp::OTDouble ) ) || ( impY->isa( BObjectImp::OTLong ) ) ) )
  {
    if ( *impX < *impY )
      return impX->copy();
    return impY->copy();
  }
  if ( auto* value = impptrIf<ObjArray>( impX ) )
  {
    if ( value->ref_arr.empty() )
      return new BError( "Array empty" );

    BObjectImp* compare = nullptr;
    for ( auto& itr : value->ref_arr )
    {
      if ( itr.get() )
      {
        BObject* bo = ( itr.get() );

        if ( bo == nullptr )
          continue;
        if ( ( bo->isa( BObjectImp::OTDouble ) ) || ( bo->isa( BObjectImp::OTLong ) ) )
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
    return new BError( "No Integer/Double elements" );
  }
  return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* MathExecutorModule::mf_Max()
{
  BObjectImp* impX = getParamImp( 0 );
  BObjectImp* impY = getParamImp( 1 );
  if ( ( ( impX->isa( BObjectImp::OTDouble ) ) || ( impX->isa( BObjectImp::OTLong ) ) ) &&
       ( ( impY->isa( BObjectImp::OTDouble ) ) || ( impY->isa( BObjectImp::OTLong ) ) ) )
  {
    if ( *impX < *impY )
      return impY->copy();
    return impX->copy();
  }
  if ( auto* value = impptrIf<ObjArray>( impX ) )
  {
    if ( value->ref_arr.empty() )
      return new BError( "Array empty" );

    BObjectImp* compare = nullptr;
    for ( auto& itr : value->ref_arr )
    {
      if ( itr.get() )
      {
        BObject* bo = ( itr.get() );

        if ( bo == nullptr )
          continue;
        if ( ( bo->isa( BObjectImp::OTDouble ) ) || ( bo->isa( BObjectImp::OTLong ) ) )
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
    return new BError( "No Integer/Double elements" );
  }
  return new BError( "Invalid parameter type" );
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
  BObjectImp* imp = getParamImp( 0 );
  if ( auto* d = impptrIf<Double>( imp ) )
  {
    return new Double( fabs( d->value() ) );
  }
  if ( auto* l = impptrIf<BLong>( imp ) )
  {
    return new BLong( labs( l->value() ) );
  }

  double x;
  // just for debug.log
  (void)getRealParam( 0, x );
  return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* MathExecutorModule::mf_Log10()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( log10( x ) );
  }

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_LogE()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( log( x ) );
  }

  return new BError( "Invalid parameter type" );
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
    snprintf( buffer, Clib::arsize( buffer ), "%.*g", static_cast<int>( digits ), x );
    return new String( buffer );
  }

  return new BError( "Invalid parameter type" );
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

  return new BError( "Invalid parameter type" );
}
Bscript::BObjectImp* MathExecutorModule::mf_Floor()
{
  double x;
  if ( getRealParam( 0, x ) )
  {
    return new Double( floor( x ) );
  }

  return new BError( "Invalid parameter type" );
}
}  // namespace Pol::Module
