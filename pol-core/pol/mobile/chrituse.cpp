/** @file
 *
 * @par History
 */


#include <stddef.h>

#include "../../bscript/bobject.h"
#include "../../bscript/eprog.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../plib/systemstate.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../module/uomod.h"
#include "../realms/realm.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../uoexec.h"
#include "../uoscrobj.h"
#include "charactr.h"

namespace Pol
{
namespace Mobile
{
bool Character::start_script( Bscript::EScriptProgram* prog, bool start_attached,
                              Bscript::BObjectImp* param2, Bscript::BObjectImp* param3,
                              Bscript::BObjectImp* param4 )
{
  if ( !( !start_attached || ( script_ex == nullptr ) ) )
  {
    POLLOGLN(
        "Character::start_script hiccup\n"
        "Trying to start script {}\n"
        "Script {} is already running",
        prog->name.get(), script_ex->scriptname() );
    return false;  // if it's going to have a passert() error, just return false.
  }
  passert( !start_attached || ( script_ex == nullptr ) );

  Bscript::BObject ob2( param2 ? param2 : Bscript::UninitObject::create() );
  Bscript::BObject ob3( param3 ? param3 : Bscript::UninitObject::create() );
  Bscript::BObject ob4( param4 ? param4 : Bscript::UninitObject::create() );

  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
  auto uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );

  if ( prog->haveProgram )
  {
    if ( param4 )
      ex->pushArg( param4 );
    if ( param3 )
      ex->pushArg( param3 );
    if ( param2 )
      ex->pushArg( param2 );
    ex->pushArg( new Module::ECharacterRefObjImp( this ) );
  }

  ex->priority( 100 );

  if ( ex->setProgram( prog ) )
  {
    if ( start_attached )
    {
      script_ex = ex.get();
      uoemod->attached_chr_ = this;
    }
    uoemod->controller_.set( this );
    schedule_executor( ex.release() );
    return true;
  }
  else
  {
    POLLOGLN( "Unable to setProgram({})", prog->name.get() );
    return false;
  }
}

bool Character::start_itemuse_script( Bscript::EScriptProgram* prog, Items::Item* item,
                                      bool start_attached )
{
  return start_script( prog, start_attached, new Module::EItemRefObjImp( item ) );
}

bool Character::start_snoop_script( Bscript::EScriptProgram* prog, Items::Item* item,
                                    Mobile::Character* owner )
{
  return start_script( prog, false, new Module::EItemRefObjImp( item ),
                       new Module::EOfflineCharacterRefObjImp( owner ) );
}
}  // namespace Mobile
namespace Items
{
void Item::walk_on( Mobile::Character* chr )
{
  const Items::ItemDesc& itemdesc = this->itemdesc();
  if ( !itemdesc.walk_on_script.empty() )
  {
    ref_ptr<Bscript::EScriptProgram> prog;
    prog = find_script2( itemdesc.walk_on_script,
                         true,  // complain if not found
                         Plib::systemstate.config.cache_interactive_scripts );
    if ( prog.get() != nullptr )
    {
      std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
      ex->addModule( new Module::UOExecutorModule( *ex ) );
      if ( prog->haveProgram )
      {
        ex->pushArg(
            new Bscript::String( chr->lastpos.realm() ? chr->lastpos.realm()->name() : "" ) );
        ex->pushArg( new Bscript::BLong( chr->lastpos.z() ) );
        ex->pushArg( new Bscript::BLong( chr->lastpos.y() ) );
        ex->pushArg( new Bscript::BLong( chr->lastpos.x() ) );
        ex->pushArg( new Module::EItemRefObjImp( this ) );
        ex->pushArg( new Module::ECharacterRefObjImp( chr ) );
      }

      ex->priority( 100 );

      if ( ex->setProgram( prog.get() ) )
      {
        schedule_executor( ex.release() );
      }
    }
  }
}
}  // namespace Items
}  // namespace Pol
