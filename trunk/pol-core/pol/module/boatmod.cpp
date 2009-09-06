/*
History
=======
2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include "../../clib/rawtypes.h"
#include "../../plib/realm.h"

#include "../multi/boat.h"
#include "boatmod.h"
#include "../item.h"
#include "../uoexhelp.h"
#include "../uoscrobj.h"
#include "../wrldsize.h"

template<>
TmplExecutorModule<UBoatExecutorModule>::FunctionDef   
    TmplExecutorModule<UBoatExecutorModule>::function_table[] = 
{
    { "MoveBoat",               &UBoatExecutorModule::mf_MoveBoat },
    { "MoveBoatRelative",       &UBoatExecutorModule::mf_MoveBoatRelative },
	{ "MoveBoatXY",				&UBoatExecutorModule::mf_MoveBoatXY},
    { "TurnBoat",               &UBoatExecutorModule::mf_TurnBoat },
    { "RegisterItemWithBoat",   &UBoatExecutorModule::mf_RegisterItemWithBoat },
    { "BoatFromItem",           &UBoatExecutorModule::mf_BoatFromItem },
    { "SystemFindBoatBySerial", &UBoatExecutorModule::mf_SystemFindBoatBySerial }
};

template<>
int TmplExecutorModule<UBoatExecutorModule>::function_table_size =
    arsize(function_table);

BObjectImp* UBoatExecutorModule::mf_MoveBoat()
{ 
    UBoat* boat = NULL;
    long direction;
    if (getUBoatParam( exec, 0, boat ) &&
        getParam( 1, direction, 0, 7 ))
    {
        UFACING facing = static_cast<UFACING>(direction);
        boat->move( facing );
        return new BLong(1);
    }
    return NULL;
}

BObjectImp* UBoatExecutorModule::mf_MoveBoatXY()
{
    UBoat* boat = NULL;
    unsigned short x,y;
    if (getUBoatParam( exec, 0, boat ) &&
        getParam( 1, x, 0, static_cast<u16>(boat->realm->width()) ) &&
		getParam( 2, y, 0, static_cast<u16>(boat->realm->height()) ))
    {
        return new BLong(boat->move_xy(x,y,0, boat->realm));
    }
    else
    {
        return new BError( "Invalid Parameter type" );
    }
}

BObjectImp* UBoatExecutorModule::mf_TurnBoat()
{
    UBoat* boat = NULL;
    long relative_dir;
    if (getUBoatParam( exec, 0, boat ) &&
        getParam( 1, relative_dir ))
    {
        relative_dir &= 3;
        return new BLong(boat->turn( static_cast<UBoat::RELATIVE_DIR>(relative_dir) ) );
    }
    else
    {
        return new BError( "Invalid Parameter type" );
    }
}

BObjectImp* UBoatExecutorModule::mf_MoveBoatRelative()
{ 
    UBoat* boat = NULL;
    long direction;
    if (getUBoatParam( exec, 0, boat ) &&
        getParam( 1, direction, 0, 7 ))
    {
        // FIXME: add bias, &= 0x7
        UFACING facing = static_cast<UFACING>((boat->boat_facing()+direction)&7);

        boat->move( facing );
        return new BLong(1);
    }
    return NULL;
}

BObjectImp* UBoatExecutorModule::mf_RegisterItemWithBoat()
{
    UBoat* boat = NULL;
    UObject* obj = NULL;
    if (getUBoatParam( exec, 0, boat ) &&
        getUObjectParam( exec, 1, obj ))
    {
        boat->register_object(obj);
        return new BLong(1);
    }
    return NULL;
}

BObjectImp* UBoatExecutorModule::mf_SystemFindBoatBySerial()
{
    UBoat* boat = NULL;
    if (getUBoatParam( exec, 0, boat ))
    {
        return boat->make_ref();
    }
    else
    {
        return new BError( "Boat not found." );
    }
}

BObjectImp* UBoatExecutorModule::mf_BoatFromItem()
{
    Item* item = NULL;
    if (getItemParam( exec, 0, item ))
    {
        if (item->ismulti())
        {
            UMulti* multi = static_cast<UMulti*>(item);
            UBoat* boat = multi->as_boat();
            if (boat != NULL)
                return boat->make_ref();
            else
                return new BError( "Multi wasn't a boat" );
        }
        else
        {
            return new BError( "Item wasn't a multi" );
        }
    }
    else
    {
        return new BError( "Invalid parameter type." );
    }
}
