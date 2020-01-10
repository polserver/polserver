/** @file
 *
 * @par History
 */


#ifndef CLILOCEMOD_H
#define CLILOCEMOD_H

#include "../polmodl.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Module
{
class ClilocExecutorModule
    : public Bscript::TmplExecutorModule<ClilocExecutorModule, Core::PolModule>
{
public:
  ClilocExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_SendSysMessageCL();
  Bscript::BObjectImp* mf_PrintTextAboveCL();
  Bscript::BObjectImp* mf_PrintTextAbovePrivateCL();
};
}  // namespace Module
}  // namespace Pol
#endif
