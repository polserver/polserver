/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 */


#ifndef BOATEMOD_H
#define BOATEMOD_H

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
class UBoatExecutorModule : public Bscript::TmplExecutorModule<UBoatExecutorModule, Core::PolModule>
{
public:
  UBoatExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_MoveBoat();
  Bscript::BObjectImp* mf_MoveBoatRelative();

  Bscript::BObjectImp* mf_TurnBoat();

  Bscript::BObjectImp* mf_RegisterItemWithBoat();
  Bscript::BObjectImp* mf_BoatFromItem();

  Bscript::BObjectImp* mf_SystemFindBoatBySerial();

  Bscript::BObjectImp* mf_MoveBoatXY();
};
}
}
#endif
