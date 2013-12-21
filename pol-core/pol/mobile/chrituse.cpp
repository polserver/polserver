/*
History
=======

Notes
=======

*/



#include "../../clib/stl_inc.h"

#include "../../clib/logfile.h"
#include "../../clib/passert.h"
#include "../module/cfgmod.h"
#include "../module/osmod.h"
#include "../module/uomod.h"
#include "../polcfg.h"
#include "../scrsched.h"
#include "../uoexec.h"
#include "../uoscrobj.h"
#include "charactr.h"
#include "../item/itemdesc.h"
#include "../scrstore.h"

namespace Pol {
  namespace Mobile {

	bool Character::start_script( Bscript::EScriptProgram* prog,
								  bool start_attached,
								  Bscript::BObjectImp* param2,
								  Bscript::BObjectImp* param3,
								  Bscript::BObjectImp* param4 )
	{
	  if ( !( !start_attached || ( script_ex == NULL ) ) )
	  {
		Clib::Log( "Character::start_script hiccup\n" );
		Clib::Log( "Trying to start script %s\n", prog->name.c_str() );
		Clib::Log( "Script %s is already running\n", script_ex->scriptname().c_str() );
		return false; // if it's going to have a passert() error, just return false.
	  }
	  passert( !start_attached || ( script_ex == NULL ) );

	  Bscript::BObject ob2( param2 ? param2 : Bscript::UninitObject::create() );
	  Bscript::BObject ob3( param3 ? param3 : Bscript::UninitObject::create() );
	  Bscript::BObject ob4( param4 ? param4 : Bscript::UninitObject::create() );

	  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
	  auto uoemod = new Module::UOExecutorModule( *ex );
	  ex->addModule( uoemod );

	  if ( prog->haveProgram )
	  {
		if ( param4 ) ex->pushArg( param4 );
		if ( param3 ) ex->pushArg( param3 );
		if ( param2 ) ex->pushArg( param2 );
		ex->pushArg( new Module::ECharacterRefObjImp( this ) );
	  }

	  ex->os_module->priority = 100;

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
		Clib::Log( "Unable to setProgram(%s)\n", prog->name.c_str() );
		return false;
	  }
	}

	bool Character::start_itemuse_script( Bscript::EScriptProgram* prog, Items::Item* item, bool start_attached )
	{
	  return start_script( prog,
						   start_attached,
						   new Module::EItemRefObjImp( item ) );
	}

  }
  namespace Items {

	void Item::walk_on( Mobile::Character* chr )
	{
	  const Items::ItemDesc& itemdesc = this->itemdesc();
	  if ( !itemdesc.walk_on_script.empty() )
	  {
		ref_ptr<Bscript::EScriptProgram> prog;
		prog = find_script2( itemdesc.walk_on_script,
							 true, // complain if not found
							 Core::config.cache_interactive_scripts );
		if ( prog.get() != NULL )
		{
		  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
		  ex->addModule( new Module::UOExecutorModule( *ex ) );
		  if ( prog->haveProgram )
		  {
			ex->pushArg( new Bscript::BLong( chr->lastz ) );
			ex->pushArg( new Bscript::BLong( chr->lasty ) );
			ex->pushArg( new Bscript::BLong( chr->lastx ) );
			ex->pushArg( new Module::EItemRefObjImp( this ) );
			ex->pushArg( new Module::ECharacterRefObjImp( chr ) );
		  }

		  ex->os_module->priority = 100;

		  if ( ex->setProgram( prog.get() ) )
		  {
			schedule_executor( ex.release() );
		  }
		}
	  }
	}
  }
}