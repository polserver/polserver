/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 */


#ifndef BOATEMOD_H
#define BOATEMOD_H

#include "../polmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class UBoatExecutorModule : public Bscript::TmplExecutorModule<UBoatExecutorModule, Core::PolModule>
{
public:
  UBoatExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_MoveBoat();
  [[nodiscard]] Bscript::BObjectImp* mf_MoveBoatRelative();

  [[nodiscard]] Bscript::BObjectImp* mf_TurnBoat();

  [[nodiscard]] Bscript::BObjectImp* mf_RegisterItemWithBoat();
  [[nodiscard]] Bscript::BObjectImp* mf_BoatFromItem();

  [[nodiscard]] Bscript::BObjectImp* mf_SystemFindBoatBySerial();

  [[nodiscard]] Bscript::BObjectImp* mf_MoveBoatXY();
};
}  // namespace Pol::Module

#endif
