/** @file
 *
 * @par History
 */


#include "basiciomod.h"
#include "bscript/berror.h"
#include "bscript/impstr.h"
#include "clib/logfacility.h"
#include "plib/systemstate.h"

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
  if ( Plib::systemstate.config.enable_colored_output && color->length() )
  {
    INFO_PRINTLN( "{}{}{}", color->value(), exec.getParamImp( 0 )->getStringRep(),
                 Clib::Logging::CONSOLE_RESET_COLOR );
  }
  else
  {
    INFO_PRINTLN( exec.getParamImp( 0 )->getStringRep() );
  }
  return new Bscript::UninitObject;
}
}  // namespace Pol::Module
