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
#include "../polcfg.h"
#include "../scrsched.h"
#include "../module/uomod.h"
#include "../uoexec.h"
#include "../uoscrobj.h"

#include "charactr.h"


bool Character::start_script( EScriptProgram* prog, 
                              bool start_attached,
                              BObjectImp* param2,
                              BObjectImp* param3,
							  BObjectImp* param4)
{
    if (! ( !start_attached || (script_ex == NULL) ) )
    {
        Log( "Character::start_script hiccup\n" );
        Log( "Trying to start script %s\n", prog->name.c_str() );
        Log( "Script %s is already running\n", script_ex->scriptname().c_str() );
		return false; // if it's going to have a passert() error, just return false.
    }
    passert( !start_attached || (script_ex == NULL) );

	BObject ob2(param2?param2:UninitObject::create());
    BObject ob3(param3?param3:UninitObject::create());
	BObject ob4(param4?param4:UninitObject::create());

    auto_ptr<UOExecutor> ex( create_script_executor() );
    UOExecutorModule* uoemod = new UOExecutorModule( *ex );
    ex->addModule( uoemod );
    
    if (prog->haveProgram)
    {
		if (param4) ex->pushArg( param4 );
        if (param3) ex->pushArg( param3 );
        if (param2) ex->pushArg( param2 );
        ex->pushArg( new ECharacterRefObjImp( this ) );
    }
    
    ex->os_module->priority = 100;

    if (ex->setProgram( prog ))
    {
        if (start_attached)
        {
            script_ex = ex.get();
            uoemod->attached_chr_ = this;
        }
        uoemod->controller_.set(this);
        schedule_executor( ex.release() );
        return true;
    }
    else
    {
        Log( "Unable to setProgram(%s)\n", prog->name.c_str() );
        return false;
    }
}

bool Character::start_itemuse_script( EScriptProgram* prog, Item* item, bool start_attached )
{
    return start_script( prog, 
                         start_attached,
                         new EItemRefObjImp( item ) );
}

#include "../item/itemdesc.h"
#include "../scrstore.h"
void Item::walk_on( Character* chr )
{
    const ItemDesc& itemdesc = find_itemdesc( objtype_ );
    if (!itemdesc.walk_on_script.empty())
    {
        ref_ptr<EScriptProgram> prog;
        prog = find_script2( itemdesc.walk_on_script,
                            true, // complain if not found
                            config.cache_interactive_scripts);
        if (prog.get() != NULL)
        {
            auto_ptr<UOExecutor> ex(create_script_executor());
            ex->addModule( new UOExecutorModule( *ex ) );
            if (prog->haveProgram)
            {
                ex->pushArg( new BLong( chr->lastz ) );
                ex->pushArg( new BLong( chr->lasty ) );
                ex->pushArg( new BLong( chr->lastx ) );
                ex->pushArg( new EItemRefObjImp( this ) );
                ex->pushArg( new ECharacterRefObjImp( chr ) );
            }

            ex->os_module->priority = 100;

            if (ex->setProgram( prog.get() ))
            {
                schedule_executor( ex.release() );
            }
        }
    }
}
