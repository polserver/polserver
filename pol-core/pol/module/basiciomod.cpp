/** @file
 *
 * @par History
 */


#include "basiciomod.h"
#include "bscript/berror.h"
#include "bscript/impstr.h"
#include "clib/logfacility.h"

#include <module_defs/basicio.h>

namespace Pol::Module
{
BasicIoExecutorModule::BasicIoExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<BasicIoExecutorModule, Bscript::ExecutorModule>( exec )
{
}

Bscript::BObjectImp* BasicIoExecutorModule::mf_Print()
{
  const Bscript::String* color;
  if ( !exec.getStringParam( 1, color ) )
    return new Bscript::BError( "Invalid parameter type" );
  if ( color->length() )
  {
    INFO_PRINT << color->value() << exec.getParamImp( 0 )->getStringRep()
               << Clib::Logging::CONSOLE_RESET_COLOR << "\n";
  }
  else
  {
    INFO_PRINT << exec.getParamImp( 0 )->getStringRep() << "\n";
  }
  return new Bscript::UninitObject;
}
}  // namespace Pol::Module
