/*
History
=======
2005/06/15 Shinigami: added CanMove - checks if an NPC can move in given direction
                      (IsLegalMove works in a different way and is used for bounding boxes only)
2006/01/18 Shinigami: set Master first and then start Script in NPC::readNpcProperties
2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
2009/03/27 MuadDib:   NPC::inform_moved() && NPC::inform_imoved()
                      split the left/entered area to fix bug where one would trigger when not enabled.
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/25 Shinigami: STLport-5.2.1 fix: params not used
                      STLport-5.2.1 fix: init order changed of damaged_sound

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include "clib/cfgelem.h"
#include "clib/clib.h"
#include "clib/endian.h"
#include "clib/fileutil.h"
#include "clib/logfile.h"
#include "clib/mlog.h"
#include "clib/passert.h"
#include "clib/random.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"
#include "clib/unicode.h"

#include "bscript/berror.h"
#include "bscript/eprog.h"
#include "bscript/executor.h"
#include "bscript/execmodl.h"
#include "bscript/modules.h"
#include "bscript/impstr.h"

#include "plib/realm.h"

#include "attribute.h"
#include "client.h"
#include "dice.h"
#include "eventid.h"
#include "fnsearch.h"
#include "realms.h"
#include "scrsched.h"
#include "listenpt.h"
#include "npc.h"
#include "npcemod.h"
#include "npctmpl.h"
#include "poltype.h"
#include "pktout.h"
#include "ufunc.h"
#include "ufuncinl.h"
#include "scrstore.h"
#include "skilladv.h"
#include "skills.h"
#include "sockio.h"
#include "ssopt.h"
#include "uvars.h"
#include "osemod.h"
#include "uoexec.h"
#include "uoemod.h"
#include "objtype.h"
#include "ufunc.h"
#include "uniemod.h"
#include "uoexhelp.h"
#include "uoscrobj.h"
#include "watch.h"
#include "weapon.h"
#include "wrldsize.h"

struct Point
{
    unsigned short x;
    unsigned short y;
};

struct Area
{
    Point topleft;
    Point bottomright;
};
class BoundingBox
{
public:
    bool contains( unsigned short x, unsigned short y ) const;
    bool read( std::string areadesc );
    void addarea( const Area& area );
private:
    typedef std::vector<Area> Areas;
    Areas areas;
};

bool BoundingBox::contains( unsigned short x, unsigned short y ) const
{
    for( Areas::const_iterator itr = areas.begin(); itr != areas.end(); itr++ )
    {
        if (x >= itr->topleft.x && x <= itr->bottomright.x &&
            y >= itr->topleft.y && y <= itr->bottomright.y)
        {
            return true;
        }
    }
    return false;
}

void BoundingBox::addarea( const Area& area )
{
    areas.push_back( area );
}



NPCExecutorModule::NPCExecutorModule( Executor& ex, NPC& npc ) :
		ExecutorModule( "NPC", ex ),
        npcref( &npc ),
		npc(npc)
{
	os_module = static_cast<OSExecutorModule*>( exec.findModule( "OS" ) );
	if (os_module == NULL)
		throw runtime_error( "NPCExecutorModule needs OS module!" );
}

NPCExecutorModule::~NPCExecutorModule()
{
    if (npc.ex == &exec)
        npc.ex = NULL;
}

NPCFunctionDef NPCExecutorModule::function_table[] =
{
    { "Wander",                 &NPCExecutorModule::mf_Wander },
    { "self",                   &NPCExecutorModule::mf_Self },
	{ "face",                   &NPCExecutorModule::face },
	{ "move",                   &NPCExecutorModule::move },
    { "WalkToward",             &NPCExecutorModule::mf_WalkToward },
    { "RunToward",              &NPCExecutorModule::mf_RunToward },
    { "WalkAwayFrom",           &NPCExecutorModule::mf_WalkAwayFrom },
    { "RunAwayFrom",            &NPCExecutorModule::mf_RunAwayFrom },
    { "TurnToward",             &NPCExecutorModule::mf_TurnToward },
    { "TurnAwayFrom",           &NPCExecutorModule::mf_TurnAwayFrom },

    { "WalkTowardLocation",     &NPCExecutorModule::mf_WalkTowardLocation },
    { "RunTowardLocation",      &NPCExecutorModule::mf_RunTowardLocation },
    { "WalkAwayFromLocation",   &NPCExecutorModule::mf_WalkAwayFromLocation },
    { "RunAwayFromLocation",    &NPCExecutorModule::mf_RunAwayFromLocation },
    { "TurnTowardLocation",     &NPCExecutorModule::mf_TurnTowardLocation },
    { "TurnAwayFromLocation",   &NPCExecutorModule::mf_TurnAwayFromLocation },


    { "say",                    &NPCExecutorModule::say },
    { "SayUC",                  &NPCExecutorModule::SayUC },
	{ "SetOpponent",            &NPCExecutorModule::mf_SetOpponent },
    { "SetWarMode",             &NPCExecutorModule::mf_SetWarMode },
    { "SetAnchor",              &NPCExecutorModule::mf_SetAnchor },
    { "position",               &NPCExecutorModule::position },
    { "facing",                 &NPCExecutorModule::facing },
    { "IsLegalMove",            &NPCExecutorModule::IsLegalMove },
    { "CanMove",                &NPCExecutorModule::CanMove },
    { "getproperty",            &NPCExecutorModule::getproperty },
    { "setproperty",            &NPCExecutorModule::setproperty },
    { "makeboundingbox",        &NPCExecutorModule::makeboundingbox }
    // { "CreateBackpack", CreateBackpack },
    // { "CreateItem", CreateItem }
};

int NPCExecutorModule::functionIndex( const char* name )
{
	for( unsigned idx = 0; idx < arsize(function_table); idx++ )
	{
		if (stricmp( name, function_table[idx].funcname ) == 0)
			return idx;
	}
	return -1;
}

BObjectImp* NPCExecutorModule::execFunc( unsigned funcidx )
{
	return callMemberFunction(*this, function_table[funcidx].fptr)();
};

static UFACING GetRandomFacing()
{
    int rval = random_int( 8 );
    return static_cast<UFACING>( rval );
}

const char* FacingStr( UFACING facing )
{
    switch( facing )
    {
        case FACING_N: return "N";
        case FACING_S: return "S";
        case FACING_E: return "E";
        case FACING_W: return "W";
        case FACING_NE: return "NE";
        case FACING_NW: return "NW";
        case FACING_SE: return "SE";
        case FACING_SW: return "SW";
    }
    return "";
}

/* IsLegalMove: parameters (move, bounding box)*/
BObjectImp* NPCExecutorModule::IsLegalMove()
{
    String* facing_str = static_cast<String*>(exec.getParamImp(0, BObjectImp::OTString));
    BApplicObjBase* appobj = static_cast<BApplicObjBase*>(exec.getParamImp(1, BObjectImp::OTApplicObj));
    if (facing_str == NULL || 
        appobj == NULL ||
        appobj->object_type() != &bounding_box_type)
    {
        return new BLong(0);
    }

    BApplicObj<BoundingBox>* ao_bbox = static_cast<BApplicObj<BoundingBox>*>(appobj);
    const BoundingBox& bbox = ao_bbox->value();

    UFACING facing;
    if (DecodeFacing( facing_str->value().c_str(), facing ) == false)
        return new BLong(0);

    unsigned short x, y;
    npc.getpos_ifmove( facing, &x, &y );

    return new BLong( bbox.contains( x, y ) );
}

/* CanMove: parameters (facing)*/
BObjectImp* NPCExecutorModule::CanMove()
{
    if (exec.fparams.size() == 1)
    {
        BObjectImp* param0 = exec.getParamImp(0);
        
        if (param0->isa( BObjectImp::OTString ))
        {
            const char* dir = exec.paramAsString(0);
            UFACING facing;
            
            if (DecodeFacing( dir, facing ) == false)
            {
                if (mlog.is_open())
                    mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
                         << "\tCall to function npc::canmove():" << endl
                         << "\tParameter 0: Expected direction: N S E W NW NE SW SE, got " << dir << endl;
                return new BError( "Invalid facing value" );
            }
            
            return new BLong(npc.could_move(facing)?1:0);
        }
        else if (param0->isa( BObjectImp::OTLong ))
        {
            BLong* blong = static_cast<BLong*>(param0);
            UFACING facing = static_cast<UFACING>(blong->value() & PKTIN_02_FACING_MASK);
            return new BLong(npc.could_move(facing)?1:0);
        }
        else
        {
            if (mlog.is_open())
                mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
                     << "\tCall to function npc::canmove():" << endl
                     << "\tParameter 0: Expected direction, got datatype "
                     << BObjectImp::typestr(param0->type()) << endl;
            return new BError( "Invalid parameter type" );
        }
    }
    else
        return new BError( "Invalid parameter count" );
}

BObjectImp* NPCExecutorModule::mf_Self()
{
    return new ECharacterRefObjImp( &npc );
}

BObjectImp* NPCExecutorModule::mf_SetAnchor()
{
    xcoord x;
    ycoord y;
    long dstart, psub;
    if (getParam( 0, x ) &&
        getParam( 1, y ) &&
        getParam( 2, dstart ) &&
        getParam( 3, psub ))
    {
		if(!npc.realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");
        if (dstart)
        {
            npc.anchor.enabled = true;
            npc.anchor.x = static_cast<unsigned short>(x);
            npc.anchor.y = static_cast<unsigned short>(y);
            npc.anchor.dstart = static_cast<unsigned short>(dstart);
            npc.anchor.psub = static_cast<unsigned short>(psub);
            return new BLong(1);
        }
        else
        {
            npc.anchor.enabled = false;
            return new BLong(1);
        }
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}


int adjustments[] =
{
    0,
    +1,
    -1,
    +2,
    -2,
    +3,
    -3
};
const int N_ADJUST = arsize( adjustments );

bool NPCExecutorModule::_internal_move(UFACING facing, int run)
{
	bool success = false;
	int dir = facing;
	if (run) 
		dir |= 0x80; // FIXME HARDCODE

	if (npc.could_move(facing))
	{
		if (npc.move(static_cast<unsigned char>(dir))) // this could still fail, if paralyzed or frozen
		{
			npc.tellmove();
			//move was successful
			success = true;
		}
		// else, npc could move, but move failed.
	}
	//else npc could not move

	return success;
}

BObjectImp* NPCExecutorModule::move_self( UFACING facing, bool run, bool adjust_ok )
{
	bool success = false;
    int dir = facing;

    if (run)
		dir |= 0x80; // FIXME HARDCODE
    
    if (adjust_ok)
    {
		if(npc.use_adjustments)
		{
			for( int i = 0; i < N_ADJUST; ++i )
			{
				facing = static_cast<UFACING>((dir + adjustments[i]) & 7);

				success = _internal_move(facing, run);
				if(success == true)
					break;
			}
		}
		else
		{
			success = _internal_move(facing, run);
		}
    }
    else
    {
		if (npc.anchor_allows_move(facing) && npc.move(static_cast<unsigned char>(dir)))
		{
			npc.tellmove();
			success = true;
		}
    }
    
    //int base = 1000 - npc.dexterity() * 3;
	int base = 1000 - npc.run_speed * 3;
    if (base < 250) base = 250;
    os_module->SleepForMs( run ? (base/2) : base );
    
    //return new String( FacingStr(facing) );
	return new BLong(success?1:0);
}

BObjectImp* NPCExecutorModule::mf_Wander()
{
    u8 newfacing = 0;
    bool adjust_ok = true;
    switch( random_int( 8 ) )
    {
        case 0: case 1: case 2: case 3: case 4: case 5:
            newfacing = npc.facing;
            break;
        case 6:
            newfacing = (static_cast<int>(npc.facing)-1) & PKTIN_02_FACING_MASK;
            adjust_ok = false;
            break;
        case 7:
            newfacing = (static_cast<int>(npc.facing)+1) & PKTIN_02_FACING_MASK;
            adjust_ok = false;
            break;
    }
    return move_self( static_cast<UFACING>(newfacing), false, adjust_ok );
}

BObjectImp* NPCExecutorModule::face()
{
	//int params = exec.numParams();
	BObjectImp* param0 = exec.getParamImp(0);
	long flags;
	exec.getParam(1, flags);
	UFACING i_facing;

	if (param0->isa( BObjectImp::OTString ))
	{
		const char* dir = exec.paramAsString(0);

		if (DecodeFacing( dir, i_facing ) == false)
		{
			if (mlog.is_open())
				mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
				<< "\tCall to function npc::face():" << endl
				<< "\tParameter 0: Expected direction: N S E W NW NE SW SE, got " << dir << endl;
			return NULL;
		}
	}
	else if (param0->isa( BObjectImp::OTLong ))
	{
		BLong* blong = static_cast<BLong*>(param0);
		i_facing = static_cast<UFACING>(blong->value() & PKTIN_02_FACING_MASK);
	}
	else
	{
		if (mlog.is_open())
			mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
			<< "\tCall to function npc::face():" << endl
			<< "\tParameter 0: Expected direction, "
			<< ", got datatype " << BObjectImp::typestr(param0->type()) << endl;

		return NULL;
	}

	if(!npc.face(i_facing, flags))
		return new BLong(0);

	npc.on_facing_changed();
	return new BLong(i_facing);
}

BObjectImp* NPCExecutorModule::move()
{
    BObjectImp* param0 = exec.getParamImp(0);

    if (param0->isa( BObjectImp::OTString ))
	{
		const char* dir = exec.paramAsString(0);
		UFACING facing;

		if (DecodeFacing( dir, facing ) == false)
        {
            if (mlog.is_open())
                mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
                     << "\tCall to function npc::move():" << endl
                     << "\tParameter 0: Expected direction: N S E W NW NE SW SE, got " << dir << endl;
            return NULL;
        }

        return move_self( facing, false );

	}
    else if (param0->isa( BObjectImp::OTLong ))
    {
        BLong* blong = static_cast<BLong*>(param0);
        UFACING facing = static_cast<UFACING>(blong->value() & PKTIN_02_FACING_MASK);
        return move_self( facing, false );
    }
    else if (param0->isa( BObjectImp::OTApplicObj ))
    {
        BApplicObjBase* appobj = static_cast<BApplicObjBase*>(param0);
        if (appobj->object_type() == &bounding_box_type)
        {
            BApplicObj<BoundingBox>* ao_bbox = static_cast<BApplicObj<BoundingBox>*>(appobj);
            const BoundingBox& bbox = ao_bbox->value();
            UFACING facing = GetRandomFacing();
            
            unsigned short x, y;
            npc.getpos_ifmove( facing, &x, &y );
            if (bbox.contains( x, y ) || !bbox.contains( npc.x, npc.y ))
            {
                npc.move( static_cast<unsigned char>(facing) );
                npc.tellmove();
                os_module->SleepFor( 1 );
                return new String( FacingStr( facing ) );
            }
            else
            {
                return new String( "" );
            }
        }
        else
        {
            if (mlog.is_open())
                mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
                     << "\tCall to function npc::move():" << endl
                     << "\tParameter 0: Expected direction or bounding box, "
                     << ", got datatype " << appobj->object_type() << endl;
    	    return NULL;
        }
    }
	else
	{
        if (mlog.is_open())
            mlog << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": " << endl
                 << "\tCall to function npc::move():" << endl
                 << "\tParameter 0: Expected direction or bounding box, "
                 << ", got datatype " << BObjectImp::typestr(param0->type()) << endl;
           
    	return NULL;
	}
}

UFACING direction_toward( const Character* src, const UObject* idst )
{
    const UObject* dst = idst->toplevel_owner();
    if (src->x < dst->x)        // East to target
    {
        if (src->y < dst->y)
            return FACING_SE;
        else if (src->y == dst->y)
            return FACING_E;
        else /* src->y > dst->y */
            return FACING_NE;
    }
    else if (src->x == dst->x)
    {
        if (src->y < dst->y)
            return FACING_S;
        else if (src->y > dst->y)
            return FACING_N;
    }
    else /* src->x > dst->x */  // West to target
    {
        if (src->y < dst->y)
            return FACING_SW;
        else if (src->y == dst->y)
            return FACING_W;
        else /* src->y > dst->y */
            return FACING_NW;
    }
    return FACING_N;
}

UFACING direction_toward( const Character* src, xcoord to_x, ycoord to_y )
{
    
    if (src->x < to_x)        // East to target
    {
        if (src->y < to_y)
            return FACING_SE;
        else if (src->y == to_y)
            return FACING_E;
        else /* src->y > dst->y */
            return FACING_NE;
    }
    else if (src->x == to_x)
    {
        if (src->y < to_y)
            return FACING_S;
        else if (src->y > to_y)
            return FACING_N;
    }
    else /* src->x > dst->x */  // West to target
    {
        if (src->y < to_y)
            return FACING_SW;
        else if (src->y == to_y)
            return FACING_W;
        else /* src->y > dst->y */
            return FACING_NW;
    }
    return FACING_N;
}

UFACING away_cvt[8] = {
    FACING_S,
    FACING_SW,
    FACING_W,
    FACING_NW,
    FACING_N,
    FACING_NE,
    FACING_E,
    FACING_SE
};

UFACING direction_away( const Character* src, const UObject* idst )
{
    UFACING toward = direction_toward(src,idst);
    UFACING away = away_cvt[ static_cast<int>(toward) ];
    return away;
}

UFACING direction_away( const Character* src, long from_x, long from_y )
{
    UFACING toward = direction_toward(src,from_x, from_y);
    UFACING away = away_cvt[ static_cast<int>(toward) ];
    return away;
}

BObjectImp* NPCExecutorModule::mf_WalkToward()
{
    UObject* obj;
    if (getUObjectParam( exec,0, obj ))
    {
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (!npc.is_visible_to_me( chr ))
                return new BError( "Mobile specified cannot be seen" );
        }
        UFACING fac = direction_toward( &npc, obj );
        return move_self( fac, false, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}


BObjectImp* NPCExecutorModule::mf_RunToward()
{
    UObject* obj;
    if (getUObjectParam( exec,0, obj ))
    {
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (!npc.is_visible_to_me( chr ))
                return new BError( "Mobile specified cannot be seen" );
        }
        return move_self( direction_toward( &npc, obj ), true, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_WalkAwayFrom()
{
    UObject* obj;
    if (getUObjectParam( exec,0, obj ))
    {
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (!npc.is_visible_to_me( chr ))
                return new BError( "Mobile specified cannot be seen" );
        }
        return move_self( direction_away( &npc, obj ), false, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_RunAwayFrom()
{
    UObject* obj;
    if (getUObjectParam( exec,0, obj ))
    {
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (!npc.is_visible_to_me( chr ))
                return new BError( "Mobile specified cannot be seen" );
        }
        return move_self( direction_away( &npc, obj ), true, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_TurnToward()
{
	UObject* obj;
	if (getUObjectParam( exec,0, obj ))
	{
		long flags;
		exec.getParam(1, flags);
		if (obj->ismobile())
		{
			Character* chr = static_cast<Character*>(obj);
			if (!npc.is_visible_to_me( chr ))
				return new BError( "Mobile specified cannot be seen" );
		}
		UFACING facing = direction_toward( &npc, obj );
		if (facing != npc.facing)
		{
			if(!npc.face( facing, flags ))
				return new BLong(0);
			npc.on_facing_changed();
			return new BLong(1);
		}
		else
			return new BLong(0);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* NPCExecutorModule::mf_TurnAwayFrom()
{
	UObject* obj;
	if (getUObjectParam( exec,0, obj ))
	{
		long flags;
		exec.getParam(1, flags);
		if (obj->ismobile())
		{
			Character* chr = static_cast<Character*>(obj);
			if (!npc.is_visible_to_me( chr ))
				return new BError( "Mobile specified cannot be seen" );
		}
		UFACING facing = direction_away( &npc, obj );
		if (facing != npc.facing)
		{
			if(!npc.face( facing, flags ))
				return new BLong(0);
			npc.on_facing_changed();
			return new BLong(1);
		}
		else
			return new BLong(0);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* NPCExecutorModule::mf_WalkTowardLocation()
{
    xcoord x;
    ycoord y;
    if (exec.getParam( 0, x ) &&
        exec.getParam( 1, y ))
    {
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
        UFACING fac = direction_toward( &npc, x, y );
        return move_self( fac, false, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_RunTowardLocation()
{
    xcoord x;
    ycoord y;
    
    if (exec.getParam( 0, x ) &&
        exec.getParam( 1, y ))
    {
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
        UFACING fac = direction_toward( &npc, x, y );
        return move_self( fac, true, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_WalkAwayFromLocation()
{
    xcoord x;
    ycoord y;
    if (exec.getParam( 0, x ) &&
        exec.getParam( 1, y ))
    {
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
        UFACING fac = direction_away( &npc, x, y );
        return move_self( fac, false, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_RunAwayFromLocation()
{
    xcoord x;
    ycoord y;
    if (exec.getParam( 0, x ) &&
        exec.getParam( 1, y ))
    {
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
        UFACING fac = direction_away( &npc, x, y );
        return move_self( fac, true, true );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::mf_TurnTowardLocation()
{
	xcoord x;
	ycoord y;
	if (exec.getParam( 0, x ) &&
		exec.getParam( 1, y ))
	{
		long flags;
		exec.getParam(2, flags);
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
		UFACING fac = direction_toward( &npc, x, y );
		if (npc.facing != fac)
		{
			if(!npc.face( fac, flags ))
				return new BLong(0);
			npc.on_facing_changed();
			return new BLong(1);
		}
		else
			return new BLong(0);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* NPCExecutorModule::mf_TurnAwayFromLocation()
{
	xcoord x;
	ycoord y;
	if (exec.getParam( 0, x ) &&
		exec.getParam( 1, y ))
	{
		long flags;
		exec.getParam(2, flags);
		if(!npc.realm->valid(x,y,npc.z)) return new BError("Invalid Coordinates for Realm");
		UFACING fac = direction_away( &npc, x, y );
		if (npc.facing != fac)
		{
			if(!npc.face( fac, flags ))
				return new BLong(0);
			npc.on_facing_changed();
			return new BLong(1);
		}
		else
			return new BLong(0);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}


BObjectImp* NPCExecutorModule::say()
{
    if ( npc.squelched() )
		return new BError("NPC is squelched");
	else if ( npc.hidden() )
		npc.unhide();
	
	const char* text = exec.paramAsString(0);
	string texttype_str = strlower(exec.paramAsString(1));
	long doevent; exec.getParam(2, doevent);

	PKTOUT_1C talkmsg;
	talkmsg.msgtype = PKTOUT_1C_ID;
	talkmsg.source_serial = npc.serial_ext;
	talkmsg.source_graphic = npc.graphic_ext;
	talkmsg.color = ctBEu16( npc.speech_color_ );
	talkmsg.font = ctBEu16( npc.speech_font_ );
	strzcpy( talkmsg.speaker_name, npc.name().c_str(), sizeof talkmsg.speaker_name );
	strzcpy( talkmsg.text, text, sizeof talkmsg.text );
	unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_1C, text ) + strlen( talkmsg.text ) + 1);
	talkmsg.msglen = ctBEu16( msglen );

	if ( texttype_str == "default" )
		talkmsg.type = TEXTTYPE_NORMAL;
	else if ( texttype_str == "whisper" )
		talkmsg.type = TEXTTYPE_WHISPER;
	else if ( texttype_str == "yell" )
		talkmsg.type = TEXTTYPE_YELL;
	else
		return new BError("texttype string param must be either 'default', 'whisper', or 'yell'");

	//cout << npc.name() << " ["<<texttype_str<<"] ["<<talkmsg.textdef.type<<"] ["<<doevent<<"] \n";

	// send to those nearby
	for( unsigned cli = 0; cli < clients.size(); cli++ )
	{
		Client* client = clients[cli];
		if ( !client->ready ) 
			continue;
		else if ( !client->chr->is_visible_to_me(&npc) )
			continue;

		bool rangeok;
		if ( talkmsg.type == TEXTTYPE_WHISPER )
			rangeok = in_whisper_range(&npc, client->chr);
		else if ( talkmsg.type == TEXTTYPE_YELL )
			rangeok = in_yell_range(&npc, client->chr);
		else
			rangeok = in_say_range(&npc, client->chr);
		
		if ( rangeok )
			transmit( client, &talkmsg, msglen );
	}
	
	if ( doevent >= 1 )
		for_nearby_npcs(npc_spoke, &npc, text, strlen(text), talkmsg.type);

    return NULL;
}

BObjectImp* NPCExecutorModule::SayUC()
{
    if ( npc.squelched() )
		return new BError("NPC is squelched");
	else if ( npc.hidden() )
		npc.unhide();
	
	ObjArray* oText;
	string texttype_str;
	const String* lang;
	long doevent;

    if (getObjArrayParam( 0, oText ) &&
		getStringParam( 2, lang ) &&
        getParam( 3, doevent))
    {
		texttype_str = strlower(exec.paramAsString(1));
		if ( texttype_str != "default" &&
			 texttype_str != "whisper" &&
			 texttype_str != "yell" )
		{
			return new BError("texttype string param must be either 'default', 'whisper', or 'yell'");
		}

		unsigned textlenucc = oText->ref_arr.size();
		if ( textlenucc > 200 )
			return new BError( "Unicode array exceeds maximum size." );
		if ( lang->length() != 3 )
			return new BError( "langcode must be a 3-character code." );
		if ( !convertArrayToUC(oText, gwtext, textlenucc) )
			return new BError( "Invalid value in Unicode array." );

		const char* languc;
		languc = strupper(lang->value()).c_str();
		static PKTOUT_AE msg;
		unsigned textlen = 0, msglen;

		//textlen = wcslen((const wchar_t*)wtext) + 1;
		while( gwtext[textlen] != L'\0' )
	    	++textlen;
		textlen += 1; //include null terminator
		if (textlen > (sizeof msg.wtext / sizeof(msg.wtext[0])))
			textlen = (sizeof msg.wtext / sizeof(msg.wtext[0]));
		msglen = offsetof( PKTOUT_AE, wtext ) + textlen*sizeof(msg.wtext[0]);

	    if (msglen > sizeof msg)
			return false;
    
		msg.msgtype = PKTOUT_AE_ID;
		msg.msglen = ctBEu16( msglen );
		msg.source_serial = npc.serial_ext;
		msg.source_graphic = npc.graphic_ext;
		if ( texttype_str == "yell" )
		{
			msg.type = TEXTTYPE_YELL;
		}
		else if ( texttype_str == "whisper" )
		{
			msg.type = TEXTTYPE_WHISPER;
		}
		else
		{
			msg.type = TEXTTYPE_NORMAL;
		}
		msg.color = ctBEu16( npc.speech_color_ );
		msg.font = ctBEu16( npc.speech_font_ );
		memcpy( &msg.lang, languc, sizeof msg.lang );
	    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
		strzcpy( msg.speaker_name, npc.description().c_str(), sizeof msg.speaker_name );
		for(unsigned i=0; i < textlen; i++)
			msg.wtext[i] = ctBEu16(gwtext[i]);
		msg.wtext[textlen] = (u16)0L;

		for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	    {
	        Client* client = *itr;
			if ( !client->ready ) 
				continue;

			bool rangeok;
			if ( msg.type == TEXTTYPE_WHISPER )
				rangeok = in_whisper_range(&npc, client->chr);
			else if ( msg.type == TEXTTYPE_YELL )
				rangeok = in_yell_range(&npc, client->chr);
			else
				rangeok = in_say_range(&npc, client->chr);
			
			if ( rangeok )
				transmit( client, &msg, msglen );

		}

	}
    else
    {
        return new BError( "A parameter was invalid" );
    }

//	if ( doevent >= 1 )
//		for_nearby_npcs( npc_spoke, &npc, ntext, ntextlen, msgin->textdef.type,
//						 wtext, msgin->lang, wtextlen);

    return NULL;
}

BObjectImp* NPCExecutorModule::position()
{
    BStruct* oa = new BStruct;
        
    oa->addMember( "x", new BLong( npc.x ) );
    oa->addMember( "y", new BLong( npc.y ) );
    oa->addMember( "z", new BLong( npc.z ) );

    return oa;
}

BObjectImp* NPCExecutorModule::facing()
{
    return new String( FacingStr( static_cast<UFACING>(npc.facing) ) );
}

BObjectImp* NPCExecutorModule::getproperty()
{
    const String* propname_str;
    if (exec.getStringParam( 0, propname_str ))
    {
        string val;
        if (npc.getprop( propname_str->value(), val ))
        {
            return BObjectImp::unpack( val.c_str() );
        }
        else
        {
            return new BError( "Property not found" );
        }
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* NPCExecutorModule::setproperty()
{
    const String* propname_str;
    if (exec.getStringParam( 0, propname_str ))
    {
        BObjectImp* propval = getParamImp( 1 );
        npc.setprop( propname_str->value(), propval->pack() );
        return new BLong(1);
    }
    else
    {
        return new BError( "Invalid parameter type" );

    }
}

BObjectImp* NPCExecutorModule::CreateBackpack()
{
    // UNTESTED
    if (!npc.layer_is_equipped( LAYER_BACKPACK ))
    {
		Item* i = Item::create( UOBJ_BACKPACK );
		i->realm = npc.realm;
        auto_ptr<Item> item( i );
        item->layer = LAYER_BACKPACK;
        if (npc.equippable(item.get()))
        {
            npc.equip( item.release() );
        }
    }
    return new BLong(1);
}

BObjectImp* NPCExecutorModule::CreateItem()
{
    // UNTESTED
    const BLong* objtype = exec.getLongParam( 0 );
    if (objtype == NULL)
        return new BLong(0);

    UContainer* backpack = npc.backpack();
    if (backpack == NULL)
        return new BLong(0);

	Item* i = Item::create( static_cast<unsigned short>(objtype->value()) );
	i->realm = npc.realm;
    auto_ptr<Item> item( i );
    if (item.get() == NULL)
        return new BLong(0);
    
    if (!backpack->can_add( *item ))
        return new BLong(0);

    u32 serial = item->serial;

	// FIXME : Add Grid Index Default Location Checks here.
	// Remember, if index fails, move to the ground.
	backpack->add( item.release() );

    return new BLong(serial);
}

class BoundingBoxObjImp : public BApplicObj<BoundingBox>
{
public:
    BoundingBoxObjImp() : BApplicObj<BoundingBox>(&bounding_box_type) {}
    explicit BoundingBoxObjImp( const BoundingBox& b ) : BApplicObj<BoundingBox>(&bounding_box_type, b) {}
    virtual const char* typeOf() const { return "BoundingBox"; }
    virtual BObjectImp* copy() const { return new BoundingBoxObjImp( value() ); }

};

/* An area definition is as follows:
   pt: (x,y)
   rect: [pt-pt]
   area: rect,rect,...
   So, format is: [(x1,y1)-(x2,y2)],[],[],...
   Well, right now, the format is x1 y1 x2 y2 ... (ick)
*/
BObjectImp* NPCExecutorModule::makeboundingbox( /* areastring */ )
{
    String* arealist = EXPLICIT_CAST(String*, BObjectImp*)(getParamImp(0,BObjectImp::OTString));
    if (arealist == NULL)
        return new String( "" );

    BoundingBoxObjImp* bbox = new BoundingBoxObjImp;
    auto_ptr< BoundingBoxObjImp > bbox_owner( bbox );
    
//    const std::string& areas = arealist->value();
    
    ISTRINGSTREAM is(arealist->value());

    Area a;
    // FIXME this is a terrible data format.
    while (is >> a.topleft.x >> a.topleft.y >> a.bottomright.x >> a.bottomright.y)
    {
        (*bbox)->addarea(a);
    }

    return bbox_owner.release();
}

BObjectImp* NPCExecutorModule::mf_SetOpponent()
{
    Character* chr;
    if (getCharacterParam( exec, 0, chr ) &&
        chr != &npc)
    {
        npc.set_opponent( chr );
        return new BLong(1);
    }
    else
    {
        npc.set_opponent( NULL );
        return new BLong(0);
    }
}

BObjectImp* NPCExecutorModule::mf_SetWarMode()
{
    long warmode;
    if (exec.getParam( 0, warmode ))
    {
        npc.set_warmode( warmode != 0 );
        return new BLong(1);
    }
    else
    {
        return new BLong(0);
    }
}

NPC::NPC( u16 objtype, const ConfigElem& elem ) :
    Character(objtype, CLASS_NPC),
	damaged_sound(0),
	ex(NULL),
	give_item_ex(NULL),
    script(""),
    npc_ar_(0),
    master_(NULL),
    template_( find_npc_template( elem ) ),
    speech_color_(DEFAULT_TEXT_COLOR),
    speech_font_(DEFAULT_TEXT_FONT)
{
	connected = 1;
    logged_in = true;
    anchor.enabled = false;
    ++npc_count;
}
 
NPC::~NPC()
{
	stop_scripts();
    --npc_count;
}

void NPC::stop_scripts()
{
    if (ex != NULL)
	{
		deschedule_executor( ex );
    	delete ex;
    	ex = NULL;
	}
}

void NPC::destroy()
{
   // stop_scripts();
    wornitems.destroy_contents();
    base::destroy();
}

const char* NPC::classname() const
{
    return "NPC";
}
#include "mdelta.h"
#include "uofile.h"
#include "uworld.h"

// 8-25-05 Austin
// Moved unsigned short pol_distance( unsigned short x1, unsigned short y1, 
//									unsigned short x2, unsigned short y2 )
// to ufunc.cpp

bool NPC::anchor_allows_move( UFACING dir ) const
{
    unsigned short newx = x + move_delta[ dir ].xmove;
    unsigned short newy = y + move_delta[ dir ].ymove;

    if (anchor.enabled && !warmode)
    {
        unsigned short curdist = pol_distance( x,    y,    anchor.x, anchor.y );
        unsigned short newdist = pol_distance( newx, newy, anchor.x, anchor.y );
        if (newdist > curdist) // if we're moving further away, see if we can
        {
            if (newdist > anchor.dstart)
            {
                int perc = 100 - (newdist-anchor.dstart)*anchor.psub;
                if (perc < 5)
                    perc = 5;
                if (random_int( 100 ) > perc)
                    return false;
            }
        }
    }
    return true;
}

bool NPC::could_move( UFACING dir ) const
{
    unsigned newx = x + move_delta[ dir ].xmove;
    unsigned newy = y + move_delta[ dir ].ymove;
    
    int newz;
    UMulti* supporting_multi;
    Item* walkon_item;
    return realm->walkheight( this, newx, newy, z, &newz, &supporting_multi, &walkon_item ) &&
           !npc_path_blocked( dir ) &&
           anchor_allows_move( dir );
}

bool NPC::npc_path_blocked( UFACING dir ) const
{
	if (cached_settings.freemove)
		return false;
    unsigned short newx = x + move_delta[ dir ].xmove;
    unsigned short newy = y + move_delta[ dir ].ymove;
        
    unsigned short wx, wy;
    zone_convert_clip( newx, newy, realm, wx, wy );

    ZoneCharacters& wchr = realm->zone[wx][wy].characters;
    for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
    {
        Character* chr = *itr;
        if (chr->x == newx &&
            chr->y == newy &&
            chr->z >= z-10 && chr->z <= z+10 &&
            !chr->dead() &&
            is_visible_to_me(chr))
        {
            return true;
        }
    }
    return false;
}

void NPC::printOn( ostream& os ) const
{ 
    os << classname() << " " << template_name << pf_endl;
    os << "{" << pf_endl;
    printProperties( os );
    os << "}" << pf_endl;
    os << pf_endl;
}

void NPC::printSelfOn( ostream& os ) const
{
   printOn( os );
}

void NPC::printProperties( std::ostream& os ) const
{
    base::printProperties( os );

    if (npc_ar_)
        os << "\tAR\t" << npc_ar_ << pf_endl;

    if (script != "")
        os << "\tscript\t" << script << pf_endl;

    if (master_.get() != NULL)
        os << "\tmaster\t" << master_->serial << pf_endl;

    if (speech_color_ != DEFAULT_TEXT_COLOR)
        os << "\tSpeechColor\t" << speech_color_ << pf_endl;

    if (speech_font_ != DEFAULT_TEXT_FONT)
        os << "\tSpeechFont\t" << speech_font_ << pf_endl;

	if (run_speed != dexterity())
        os << "\tRunSpeed\t" << run_speed << pf_endl;

	if (use_adjustments != true)
        os << "\tUseAdjustments\t" << use_adjustments << pf_endl;

	if (element_resist.fire != 0)
		os << "\tFireResist\t" << static_cast<int>(element_resist.fire) << pf_endl;
	if (element_resist.cold  != 0)
		os << "\tColdResist\t" << static_cast<int>(element_resist.cold) << pf_endl;
	if (element_resist.energy != 0)
		os << "\tEnergyResist\t" << static_cast<int>(element_resist.energy) << pf_endl;
	if (element_resist.poison != 0)
		os << "\tPoisonResist\t" << static_cast<int>(element_resist.poison) << pf_endl;
	if (element_resist.physical != 0)
		os << "\tPhysicalResist\t" << static_cast<int>(element_resist.physical) << pf_endl;

	if (element_damage.fire != 0)
		os << "\tFireDamage\t" << static_cast<int>(element_damage.fire) << pf_endl;
	if (element_damage.cold  != 0)
		os << "\tColdDamage\t" << static_cast<int>(element_damage.cold) << pf_endl;
	if (element_damage.energy != 0)
		os << "\tEnergyDamage\t" << static_cast<int>(element_damage.energy) << pf_endl;
	if (element_damage.poison != 0)
		os << "\tPoisonDamage\t" << static_cast<int>(element_damage.poison) << pf_endl;
	if (element_damage.physical != 0)
		os << "\tPhysicalDamage\t" << static_cast<int>(element_damage.physical) << pf_endl;

}

void NPC::printDebugProperties( std::ostream& os ) const
{
    base::printDebugProperties( os );
    os << "# template: " << template_.name << pf_endl;
    if (anchor.enabled)
    {
        os << "# anchor: x=" << anchor.x 
           << " y=" << anchor.y 
           << " dstart=" << anchor.dstart 
           << " psub=" << anchor.psub << pf_endl;
    }
}

void NPC::readNpcProperties( ConfigElem& elem )
{
    UWeapon* wpn = find_intrinsic_weapon( elem.rest() );
    if (wpn == NULL)
    {
        wpn = create_intrinsic_weapon_from_npctemplate( elem, template_.pkg );
    }
    if ( wpn != NULL )
        weapon = wpn;
    
	for (int i = 0; i < 6; i++)
	{
		loadResistances( i, elem);
		if ( i > 0)
			loadDamages( i, elem);
	}

	//dave 3/19/3, read templatename only if empty
	if(template_name.empty())
	{
		template_name = elem.rest();
    
		if (template_name == "")
		{
			string tmp;
			if (getprop( "template", tmp))
			{
				template_name = tmp.c_str()+1; 
			}
		}
	}

    unsigned long master_serial;
    if (elem.remove_prop( "MASTER", &master_serial ))
    {
		Character* chr = system_find_mobile(  master_serial );
        if (chr != NULL)
            master_.set( chr );
    }

    script = elem.remove_string( "script", "" );
    if (!script.empty())
        start_script();

    speech_color_ = elem.remove_ushort( "SpeechColor", DEFAULT_TEXT_COLOR );
    speech_font_ = elem.remove_ushort( "SpeechFont", DEFAULT_TEXT_FONT );

	use_adjustments = elem.remove_bool( "UseAdjustments", true );
	run_speed = elem.remove_ushort( "RunSpeed", dexterity() );

	damaged_sound = elem.remove_unsigned("DamagedSound", 0);
}

// This now handles all resistances, including AR to simplify the code.
void NPC::loadResistances( int resistanceType, ConfigElem& elem )
{
	string tmp;
	bool passed = false;
    Dice dice;
    string errmsg;
	// 0 = AR
	// 1 = Fire
	// 2 = Cold
	// 3 = Energy
	// 4 = Poison
	// 5 = Physical
	switch(resistanceType)
	{
		case 0: passed = elem.remove_prop( "AR", &tmp ); break;
		case 1: passed = elem.remove_prop( "FIRERESIST", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDRESIST", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYRESIST", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONRESIST", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALRESIST", &tmp ); break;
	}

    if (passed)
    {
        if (!dice.load( tmp.c_str(), &errmsg ))
        {
            cerr << "Error reading resistance "<< resistanceType << " for NPC: " << errmsg << endl;
            throw runtime_error( "Error loading NPC" );
        }
		switch(resistanceType)
		{
			case 0: npc_ar_ = dice.roll(); break;
			case 1: element_resist.fire = dice.roll(); break;
			case 2: element_resist.cold = dice.roll(); break;
			case 3: element_resist.energy = dice.roll(); break;
			case 4: element_resist.poison = dice.roll(); break;
			case 5: element_resist.physical = dice.roll(); break;
		}
    }
    else
    {
		switch(resistanceType)
		{
			case 0:  npc_ar_ = 0; break;
			case 1: element_resist.fire = 0; break;
			case 2: element_resist.cold = 0; break;
			case 3: element_resist.energy = 0; break;
			case 4: element_resist.poison = 0; break;
			case 5: element_resist.physical = 0; break;
		}
    }
}

// This now handles all resistances, including AR to simplify the code.
void NPC::loadDamages( int damageType, ConfigElem& elem )
{
	string tmp;
	bool passed = false;
    Dice dice;
    string errmsg;
	// 1 = Fire
	// 2 = Cold
	// 3 = Energy
	// 4 = Poison
	// 5 = Physical
	switch(damageType)
	{
		case 1: passed = elem.remove_prop( "FIREDAMAGE", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDDAMAGE", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYDAMAGE", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONDAMAGE", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALDAMAGE", &tmp ); break;
	}

    if (passed)
    {
        if (!dice.load( tmp.c_str(), &errmsg ))
        {
            cerr << "Error reading damage "<< damageType << " for NPC: " << errmsg << endl;
            throw runtime_error( "Error loading NPC" );
        }
		switch(damageType)
		{
			case 1: element_damage.fire = dice.roll(); break;
			case 2: element_damage.cold = dice.roll(); break;
			case 3: element_damage.energy = dice.roll(); break;
			case 4: element_damage.poison = dice.roll(); break;
			case 5: element_damage.physical = dice.roll(); break;
		}
    }
    else
    {
		switch(damageType)
		{
			case 1: element_damage.fire = 0; break;
			case 2: element_damage.cold = 0; break;
			case 3: element_damage.energy = 0; break;
			case 4: element_damage.poison = 0; break;
			case 5: element_damage.physical = 0; break;
		}
    }
}

void NPC::readProperties( ConfigElem& elem )
{
	//3/18/3 dave copied this npctemplate code from readNpcProperties, because base::readProperties 
	//will call the exported vital functions before npctemplate is set (distro uses npctemplate in the exported funcs).
    template_name = elem.rest();
    
    if (template_name == "")
    {
        string tmp;
        if (getprop( "template", tmp))
        {
            template_name = tmp.c_str()+1; 
        }
    }
    base::readProperties( elem );
    readNpcProperties( elem );
}

void NPC::readNewNpcAttributes( ConfigElem& elem )
{
    string diestring;
    Dice dice;
    string errmsg;

    for( Attribute* pAttr = FindAttribute(0); pAttr; pAttr = pAttr->next )
    {
        AttributeValue& av = attribute(pAttr->attrid);
        for( unsigned i = 0; i < pAttr->aliases.size(); ++i )
        {
            if (elem.remove_prop( pAttr->aliases[i].c_str(), &diestring ))
            {
                if (!dice.load( diestring.c_str(), &errmsg ) )
                {
                    elem.throw_error( "Error reading Attribute "
                                + pAttr->name +
                                ": " + errmsg );
                }
                long base = dice.roll() * 10;
                if (base > static_cast<long>(ATTRIBUTE_MAX_BASE))
                    base = ATTRIBUTE_MAX_BASE;

                av.base( static_cast<unsigned short>(base) );

                break;
            }
        }
    }
}

void NPC::readPropertiesForNewNPC( ConfigElem& elem )
{
    readCommonProperties( elem );
    readNewNpcAttributes( elem );
    readNpcProperties( elem );
    calc_vital_stuff();
    set_vitals_to_maximum();

//    readNpcProperties( elem );
}

void NPC::restart_script()
{
    if (ex != NULL)
    {
        ex->seterror( true );
        ex = NULL;
        // when the NPC executor module destructs, it checks this NPC to see if it points
        // back at it.  If not, it leaves us alone.
    }
    if (!script.empty())
        start_script();
}

void NPC::on_death( Item* corpse )
{
    // base::on_death intentionally not called
    send_remove_character_to_nearby( this );


    corpse->setprop( "npctemplate", "s" + template_name );
	if (FileExists("scripts/misc/death.ecl"))
		::start_script( "misc/death", new EItemRefObjImp( corpse ) );
    
    ClrCharacterWorldPosition( this, "NPC death" );
    if (ex != NULL)
    {
        // this will force the execution engine to stop running this script immediately
        ex->seterror(true);
    }
    
    destroy();
}


void NPC::start_script()
{
    passert( ex == NULL );
    passert( !script.empty() );
    ScriptDef sd( script, template_.pkg, "scripts/ai/" );
    // Log( "NPC script starting: %s\n", sd.name().c_str() );

    ref_ptr<EScriptProgram> prog = find_script2( sd );
        // find_script( "ai/" + script );
    
    if (prog.get() == NULL)
    {
        cerr << "Unable to read script " << sd.name() 
             << " for NPC " << name() << "(" << hexint(serial) << ")" << endl;
        throw runtime_error( "Error loading NPCs" );
    }

	ex = create_script_executor();
	ex->addModule( new NPCExecutorModule( *ex, *this ) );
    UOExecutorModule* uoemod = new UOExecutorModule( *ex );
    ex->addModule( uoemod );
	if (ex->setProgram( prog.get() ) == false)
    {
        cerr << "There was an error running script " << script << " for NPC "
            << name() << "(0x" << hex << serial << dec << ")" << endl;
        throw runtime_error( "Error loading NPCs" );
    }

	uoemod->attached_npc_ = this;

	schedule_executor( ex );
}


bool NPC::can_be_renamed_by( const Character* chr ) const
{
    return (master_.get() == chr);
}

void NPC::on_give_item()
{
}



void NPC::on_pc_spoke( Character* src_chr, const char* speech, u8 texttype)
{
    /*
    cerr << "PC " << src_chr->name()
         << " spoke in range of NPC " << name() 
         << ": '" << speech << "'" << endl;
         */

    if (ex != NULL)
    {
		if ((ex->eventmask & EVID_SPOKE) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ) )
				ex->os_module->signal_event( new SpeechEvent( src_chr, speech,
															  TextTypeToString(texttype) ) ); //DAVE added texttype
        }
    }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype)
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_GHOST_SPEECH) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ) )
				ex->os_module->signal_event( new SpeechEvent( src_chr, speech,
															  TextTypeToString(texttype) ) ); //DAVE added texttype
        }
    }
}

void NPC::on_pc_spoke( Character *src_chr, const char *speech, u8 texttype,
					   const u16* wspeech, const char lang[4])
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_SPOKE) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ))
				ex->os_module->signal_event( new UnicodeSpeechEvent( src_chr, speech,
																	 TextTypeToString(texttype),
																	 wspeech, lang ) );
        }
    }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype,
							 const u16* wspeech, const char lang[4])
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_GHOST_SPEECH) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ))
				ex->os_module->signal_event( new UnicodeSpeechEvent( src_chr, speech,
																	 TextTypeToString(texttype),
																	 wspeech, lang ) );
        }
    }
}

void NPC::inform_engaged( Character* engaged )
{
    // someone has targetted us. Create an event if appropriate.
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_ENGAGED)
        {
            ex->os_module->signal_event( new EngageEvent( engaged ) );
        }
    }
    // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_disengaged( Character* disengaged )
{
    // someone has targetted us. Create an event if appropriate.
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_DISENGAGED)
        {
            ex->os_module->signal_event( new DisengageEvent( disengaged ) );
        }
    }
    // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_criminal( Character* thecriminal )
{
    if (ex != NULL)
    {
        if ((ex->eventmask & (EVID_GONE_CRIMINAL)) && inrangex( this, thecriminal, ex->area_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( thecriminal ))
                ex->os_module->signal_event( new SourcedEvent( EVID_GONE_CRIMINAL, thecriminal ) );
        }
	}
}

void NPC::inform_leftarea( Character* wholeft )
{
	if (ex != NULL)
    {
        if ( ex->eventmask & (EVID_LEFTAREA) )
        {
			if ( pol_distance(this, wholeft) <= ex->area_size )
                ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, wholeft ) );
        }
	}
}

void NPC::inform_moved( Character* moved )
{
	// 8-26-05    Austin
	// Note: This does not look at realms at all, just X Y coords.

    if (ex != NULL)
    {
        if ((moved == opponent_) && (ex->eventmask & (EVID_OPPONENT_MOVED)))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                ex->os_module->signal_event( new SourcedEvent( EVID_OPPONENT_MOVED, moved ) );
        }
        else if ( ex->eventmask & (EVID_ENTEREDAREA) )
        {
            passert( this != NULL );
            passert( moved != NULL );
                // egcs may have a compiler bug when calling these as inlines
            bool are_inrange = (abs( x - moved->x ) <= ex->area_size) &&
                               (abs( y - moved->y ) <= ex->area_size);

                // inrangex_inline( this, moved, ex->area_size );
            bool were_inrange =(abs( x - moved->lastx ) <= ex->area_size) &&
		                       (abs( y - moved->lasty ) <= ex->area_size);
                
                // inrangex_inline( this, moved->lastx, moved->lasty, ex->area_size );
            if ( are_inrange && !were_inrange && (ex->eventmask & (EVID_ENTEREDAREA)) )
            {
                if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_ENTEREDAREA, moved ) );
            }
        }
        else if ( ex->eventmask & (EVID_LEFTAREA) )
        {
            passert( this != NULL );
            passert( moved != NULL );
                // egcs may have a compiler bug when calling these as inlines
            bool are_inrange = (abs( x - moved->x ) <= ex->area_size) &&
                               (abs( y - moved->y ) <= ex->area_size);

                // inrangex_inline( this, moved, ex->area_size );
            bool were_inrange =(abs( x - moved->lastx ) <= ex->area_size) &&
		                       (abs( y - moved->lasty ) <= ex->area_size);
                
            if ( !are_inrange && were_inrange && (ex->eventmask & (EVID_LEFTAREA)) )
            {
				if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, moved ) );
            }
		}
	}
}

//
// This NPC moved.  Tell him about other mobiles that have "entered" his area
// (through his own movement) 
//

void NPC::inform_imoved( Character* chr )
{
    if ( ex != NULL )
    {
        passert( this != NULL );
        passert( chr != NULL );

		// egcs may have a compiler bug when calling these as inlines
        bool are_inrange = (abs( x - chr->x ) <= ex->area_size) &&
							(abs( y - chr->y ) <= ex->area_size);
            
        bool were_inrange =(abs( lastx - chr->x ) <= ex->area_size) &&
							(abs( lasty - chr->y ) <= ex->area_size);

		if ( ex->eventmask & (EVID_ENTEREDAREA) )
        {
			if ( are_inrange && !were_inrange )
            {
               if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( chr ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_ENTEREDAREA, chr ) );
            }
        }
        else if ( ex->eventmask & (EVID_LEFTAREA) )
        {
            if (!are_inrange && were_inrange)
            {
                if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( chr ))
                        ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, chr ) );
            }
        }
	}
}

bool NPC::can_accept_event( EVENTID eventid )
{
    if (ex == NULL)
        return false;
    if (ex->eventmask & eventid)
        return true;
    else
        return false;
}

BObjectImp* NPC::send_event( BObjectImp* event )
{
    if (ex != NULL)
    {
        if (ex->os_module->signal_event( event ))
			return new BLong(1);
		else
			return new BError( "Event queue is full, discarding event" );
    }
    else
    {
        BObject bo( event );
        return new BError( "That NPC doesn't have a control script" );
    }
}

void NPC::apply_raw_damage_hundredths( unsigned long damage, Character* source )
{
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_DAMAGED)
        {
            ex->os_module->signal_event( new DamageEvent( source, static_cast<unsigned short>(damage/100) ) );
        }
    }

    base::apply_raw_damage_hundredths( damage, source );
}

/*
void NPC::on_swing_failure( Character* attacker )
{
    base::on_swing_failure(attacker);
}
*/

// keep this in sync with Character::armor_absorb_damage
double NPC::armor_absorb_damage( double damage )
{
    if (!npc_ar_)
    {
        return base::armor_absorb_damage( damage );
    }
    else
    {
        int blocked = npc_ar_ + ar_mod_;
        if (blocked < 0) blocked = 0;
        int absorbed = blocked / 2;
        
        blocked -= absorbed;
        absorbed += random_int( blocked+1 );
        if (watch.combat) cout << absorbed << " hits absorbed by NPC armor." << endl;
        damage -= absorbed;
        if (damage < 0)
            damage = 0;
    }
    return damage;
}

unsigned short calc_thru_damage( double damage, unsigned short ar );

void NPC::get_hitscript_params( double damage,
                                UArmor** parmor,
                                unsigned short* rawdamage )
{
    if (!npc_ar_)
    {
        base::get_hitscript_params( damage, parmor, rawdamage );
    }
    else
    {
        *rawdamage = static_cast<unsigned short>(calc_thru_damage( damage, npc_ar_ + ar_mod_ ));
    }
}

UWeapon* NPC::intrinsic_weapon()
{
    if (template_.intrinsic_weapon)
        return template_.intrinsic_weapon;
    else
        return wrestling_weapon;
}
