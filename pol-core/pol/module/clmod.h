/** @file
 *
 * @par History
 */


#ifndef CLILOCEMOD_H
#define CLILOCEMOD_H

#include "../../bscript/execmodl.h"
namespace Pol
{
namespace Module
{
class ClilocExecutorModule : public Bscript::TmplExecutorModule<ClilocExecutorModule>
{
public:
  ClilocExecutorModule( Bscript::Executor& exec )
      : Bscript::TmplExecutorModule<ClilocExecutorModule>( "cliloc", exec ){};

  Bscript::BObjectImp* mf_SendSysMessageCL();
  Bscript::BObjectImp* mf_PrintTextAboveCL();
  Bscript::BObjectImp* mf_PrintTextAbovePrivateCL();
};
}
}

#endif
