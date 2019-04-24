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

#include "../../bscript/execmodl.h"
#include "../../plib/uconst.h"
#include "../reftypes.h"

namespace Pol
{
namespace Bscript
{
class Executor;
template <class T>
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

class NPCExecutorModule : public Bscript::TmplExecutorModule<NPCExecutorModule>
{
public:
  NPCExecutorModule( Bscript::Executor& ex, Mobile::NPC& npc );
  virtual ~NPCExecutorModule();

  Core::NpcRef npcref;
  Mobile::NPC& npc;

protected:
  OSExecutorModule* os_module;

  friend class Bscript::TmplExecutorModule<NPCExecutorModule>;

  Bscript::BObjectImp* mf_Wander();
  Bscript::BObjectImp* mf_Self();
  Bscript::BObjectImp* face();
  Bscript::BObjectImp* move();
  Bscript::BObjectImp* mf_WalkToward();
  Bscript::BObjectImp* mf_RunToward();
  Bscript::BObjectImp* mf_WalkAwayFrom();
  Bscript::BObjectImp* mf_RunAwayFrom();
  Bscript::BObjectImp* mf_TurnToward();
  Bscript::BObjectImp* mf_TurnAwayFrom();

  Bscript::BObjectImp* mf_WalkTowardLocation();
  Bscript::BObjectImp* mf_RunTowardLocation();
  Bscript::BObjectImp* mf_WalkAwayFromLocation();
  Bscript::BObjectImp* mf_RunAwayFromLocation();
  Bscript::BObjectImp* mf_TurnTowardLocation();
  Bscript::BObjectImp* mf_TurnAwayFromLocation();

  Bscript::BObjectImp* say();
  Bscript::BObjectImp* SayUC();
  Bscript::BObjectImp* position();
  Bscript::BObjectImp* facing();
  Bscript::BObjectImp* getproperty( /* propertyname */ );
  Bscript::BObjectImp* setproperty( /* propertyname propertyvalue */ );
  Bscript::BObjectImp* makeboundingbox( /* areastring */ );
  Bscript::BObjectImp* IsLegalMove();
  Bscript::BObjectImp* CanMove();
  Bscript::BObjectImp* CreateBackpack();
  Bscript::BObjectImp* CreateItem();
  Bscript::BObjectImp* mf_SetOpponent();
  Bscript::BObjectImp* mf_SetWarMode();
  Bscript::BObjectImp* mf_SetAnchor();

  Bscript::BObjectImp* move_self( Plib::UFACING facing, bool run, bool adjust_ok = false );

  virtual size_t sizeEstimate() const override;

private:
  bool _internal_move( Plib::UFACING facing, int run );
};
}  // namespace Module
}  // namespace Pol
#endif
