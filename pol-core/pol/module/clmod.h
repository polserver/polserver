/** @file
 *
 * @par History
 */


#ifndef CLILOCEMOD_H
#define CLILOCEMOD_H

#include "../polmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class ClilocExecutorModule
    : public Bscript::TmplExecutorModule<ClilocExecutorModule, Core::PolModule>
{
public:
  ClilocExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_SendSysMessageCL();
  [[nodiscard]] Bscript::BObjectImp* mf_PrintTextAboveCL();
  [[nodiscard]] Bscript::BObjectImp* mf_PrintTextAbovePrivateCL();
};
}  // namespace Pol::Module

#endif
