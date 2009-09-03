/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/cfgelem.h"
#include "clib/fileutil.h"

#include "bscript/impstr.h"

#include "charactr.h"
#include "guardrgn.h"
#include "polcfg.h"
#include "scrdef.h"
#include "scrstore.h"
#include "scrsched.h"
#include "uoscrobj.h"

#include "client.h"
#include "cgdata.h"

JusticeRegion::JusticeRegion( ConfigElem& elem ) : 
	Region(elem),
	guarded_(elem.remove_bool( "Guarded", false )),
	nocombat_(elem.remove_bool( "NoCombat", false)),
	region_name_(elem.rest()),
	entertext_(elem.remove_string( "EnterText", "" )),
	leavetext_(elem.remove_string( "LeaveText", "" )),
	enter_script_(elem.remove_string("EnterScript", "")),
	leave_script_(elem.remove_string("LeaveScript", ""))
{
}
JusticeDef* justicedef;

void read_justice_zones()
{
	justicedef = new JusticeDef( "Justice" );
    read_region_data( *justicedef, 
                      "regions/justice.cfg", // preferred
                      "regions/regions.cfg", // other
                      "JusticeRegion Region" );


}

bool JusticeRegion::RunEnterScript(Character* chr)
{
	if ( enter_script_.empty() )
	{
		return false;
	}

	ScriptDef sd(enter_script_, 0);
	if ( !sd.exists() )
		return false;
    
	BObjectImp* res = run_script_to_completion(sd, new ECharacterRefObjImp(chr), new String(region_name_));
	return res->isTrue();
}

bool JusticeRegion::RunLeaveScript(Character* chr)
{
	if ( leave_script_.empty() )
	{
		return false;
	}

	ScriptDef sd(leave_script_, 0);
	if ( !sd.exists() )
		return false;
    
	BObjectImp* res = run_script_to_completion(sd, new ECharacterRefObjImp(chr), new String(region_name_));
	return res->isTrue();
}

bool JusticeRegion::RunNoCombatCheck(Client* client)
{
	JusticeRegion* cur_justice_region = client->gd->justice_region;

	if ( (cur_justice_region != NULL) && cur_justice_region->nocombat_ != false )
	{
		return true;
	} else {
		return false;
	}
}
