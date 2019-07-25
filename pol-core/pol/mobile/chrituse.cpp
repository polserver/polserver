/** @file
 *
 * @par History
 */


#include <stddef.h>

#include "../../bscript/bobject.h"
#include "../../bscript/eprog.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../plib/systemstate.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../module/uomod.h"
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
bool Character::start_script( Bscript::Program* prog, bool start_attached,
                              Bscript::BObjectImp* param2, Bscript::BObjectImp* param3,
                              Bscript::BObjectImp* param4 )
{
  if ( !( !start_attached || ( script_ex == nullptr ) ) )
  {
    POLLOG << "Character::start_script hiccup\n"
           << "Trying to start script " << prog->scriptname() << "\n"
           << "Script " << script_ex->scriptname() << " is already running\n";
    return false;  // if it's going to have a passert() error, just return false.
  }
  passert( !start_attached || ( script_ex == nullptr ) );

  Bscript::BObject ob2( param2 ? param2 : Bscript::UninitObject::create() );
  Bscript::BObject ob3( param3 ? param3 : Bscript::UninitObject::create() );
  Bscript::BObject ob4( param4 ? param4 : Bscript::UninitObject::create() );

  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
  auto uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );

  if ( prog->hasProgram() )
  {
    if ( param4 )
      ex->pushArg( param4 );
    if ( param3 )
      ex->pushArg( param3 );
    if ( param2 )
      ex->pushArg( param2 );
    ex->pushArg( new Module::ECharacterRefObjImp( this ) );
  }

  if ( ex->setProgram( prog ) )
  {
    ex->priority( 100 );
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
    POLLOG << "Unable to setProgram(" << prog->scriptname() << ")\n";
    return false;
  }
}

bool Character::start_itemuse_script( Bscript::Program* prog, Items::Item* item,
                                      bool start_attached )
{
  return start_script( prog, start_attached, new Module::EItemRefObjImp( item ) );
}
}
namespace Items
{
void Item::walk_on( Mobile::Character* chr )
{
  const Items::ItemDesc& itemdesc = this->itemdesc();
  if ( !itemdesc.walk_on_script.empty() )
  {
    ref_ptr<Bscript::Program> prog;
    prog = find_script2( itemdesc.walk_on_script,
                         true,  // complain if not found
                         Plib::systemstate.config.cache_interactive_scripts );
    if ( prog.get() != nullptr )
    {
      std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
      ex->addModule( new Module::UOExecutorModule( *ex ) );
      if ( prog->hasProgram() )
      {
        ex->pushArg( new Bscript::BLong( chr->lastz ) );
        ex->pushArg( new Bscript::BLong( chr->lasty ) );
        ex->pushArg( new Bscript::BLong( chr->lastx ) );
        ex->pushArg( new Module::EItemRefObjImp( this ) );
        ex->pushArg( new Module::ECharacterRefObjImp( chr ) );
      }

      if ( ex->setProgram( prog.get() ) )
      {
        ex->priority( 100 );
        schedule_executor( ex.release() );
      }
    }
  }
}
}
}
