/** @file
 *
 * @par History
 * - 2005/06/15 Shinigami: added CanMove - checks if an NPC can move in given direction
 *                         (IsLegalMove works in a different way and is used for bounding boxes
 * only)
 */


#ifndef NPCEMOD_H
#define NPCEMOD_H

#include <string>

#include "../base/position.h"
#include "../polmodl.h"
#include "../reftypes.h"

namespace Pol
{
namespace Bscript
{
class Executor;
template <class T, class T2>
class TmplExecutorModule;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Mobile
{
class NPC;
}

namespace Module
{
class OSExecutorModule;

class NPCExecutorModule : public Bscript::TmplExecutorModule<NPCExecutorModule, Core::PolModule>
{
public:
  NPCExecutorModule( Bscript::Executor& ex, Mobile::NPC& npc );
  virtual ~NPCExecutorModule();

  const Mobile::NPC& controlled_npc() const;

public:
  [[nodiscard]] Bscript::BObjectImp* mf_Wander();
  [[nodiscard]] Bscript::BObjectImp* mf_Self();
  [[nodiscard]] Bscript::BObjectImp* mf_Face();
  [[nodiscard]] Bscript::BObjectImp* mf_Move();
  [[nodiscard]] Bscript::BObjectImp* mf_WalkToward();
  [[nodiscard]] Bscript::BObjectImp* mf_RunToward();
  [[nodiscard]] Bscript::BObjectImp* mf_WalkAwayFrom();
  [[nodiscard]] Bscript::BObjectImp* mf_RunAwayFrom();
  [[nodiscard]] Bscript::BObjectImp* mf_TurnToward();
  [[nodiscard]] Bscript::BObjectImp* mf_TurnAwayFrom();
  
  [[nodiscard]] Bscript::BObjectImp* mf_WalkTowardLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_RunTowardLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_WalkAwayFromLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_RunAwayFromLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_TurnTowardLocation();
  [[nodiscard]] Bscript::BObjectImp* mf_TurnAwayFromLocation();
  
  [[nodiscard]] Bscript::BObjectImp* mf_Say();
  [[nodiscard]] Bscript::BObjectImp* mf_SayUC();
  [[nodiscard]] Bscript::BObjectImp* mf_position();
  [[nodiscard]] Bscript::BObjectImp* mf_Facing();
  [[nodiscard]] Bscript::BObjectImp* mf_GetProperty( /* propertyname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SetProperty( /* propertyname propertyvalue */ );
  [[nodiscard]] Bscript::BObjectImp* mf_MakeBoundingBox( /* areastring */ );
  [[nodiscard]] Bscript::BObjectImp* mf_IsLegalMove();
  [[nodiscard]] Bscript::BObjectImp* mf_CanMove();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateBackpack();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateItem();
  [[nodiscard]] Bscript::BObjectImp* mf_SetOpponent();
  [[nodiscard]] Bscript::BObjectImp* mf_SetWarMode();
  [[nodiscard]] Bscript::BObjectImp* mf_SetAnchor();

protected:
  OSExecutorModule* os_module;

  Core::NpcRef npcref;
  Mobile::NPC& npc;

  Bscript::BObjectImp* move_self( Core::UFACING facing, bool run, bool adjust_ok = false );

  virtual size_t sizeEstimate() const override;

private:
  bool _internal_move( Core::UFACING facing, int run );
};

inline const Mobile::NPC& NPCExecutorModule::controlled_npc() const
{
  return const_cast<const Mobile::NPC&>( this->npc );
}

}  // namespace Module
}  // namespace Pol
#endif
