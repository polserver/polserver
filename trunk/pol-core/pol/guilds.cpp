/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/09/27 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2007/06/17 Shinigami: added config.world_data_path

Notes
=======

*/


#include "clib/stl_inc.h"
#include "bscript/execmodl.h"
#include "bscript/berror.h"
#include "bscript/bobject.h"
#include "bscript/impstr.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/cfgsect.h"
#include "clib/fileutil.h"
#include "clib/refptr.h"
#include "clib/stlutil.h"

#include "charactr.h"
#include "fnsearch.h"
#include "guilds.h"
#include "polcfg.h"
#include "proplist.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoemod.h"
#include "uoscrobj.h"

/// Guild Object
///  Properties:
///   guild.guildid : integer
///   guild.members : array of offline mobilerefs
///   guild.allyguilds  : array of allied guilds
///   guild.enemyguilds : array of enemy guilds
///
///  Methods:
///   guild.ismember( who )
///   guild.isallyguild( guild )
///   guild.isenemyguild(guild)
///
///   guild.addmember( who )
///   guild.addallyguild( guild )
///   guild.addenemyguild( guild )
///
///   guild.removemember( who )
///   guild.removeallyguild( guild )
///   guild.removeenemyguild( guild )
///
///   guild.getprop( propname )
///   guild.setprop( propname, propvalue )
///   guild.eraseprop( propname )
///

template<>
TmplExecutorModule<GuildExecutorModule>::FunctionDef
	TmplExecutorModule<GuildExecutorModule>::function_table[] =
{
	{ "ListGuilds",				&GuildExecutorModule::mf_ListGuilds },
	{ "CreateGuild",			&GuildExecutorModule::mf_CreateGuild },
	{ "FindGuild",				&GuildExecutorModule::mf_FindGuild },
	{ "DestroyGuild",			&GuildExecutorModule::mf_DestroyGuild },
};
template<>
int TmplExecutorModule<GuildExecutorModule>::function_table_size =
	arsize(function_table);


typedef ref_ptr<Guild> GuildRef;

class EGuildRefObjImp : public BApplicObj<GuildRef>
{
public:
    EGuildRefObjImp( GuildRef gref );
    virtual const char* typeOf() const;
    virtual BObjectImp* copy() const;
    virtual bool isTrue() const;
	virtual bool isEqual(const BObjectImp& objimp) const;

    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
};

typedef map<unsigned long, GuildRef> Guilds;

static Guilds guilds;
static unsigned long nextguildid = 1;


SerialSet::SerialSet( ConfigElem& elem, const char* tag )
{
    unsigned long tmp;
    while (elem.remove_prop( tag, &tmp ))
    {
        insert( tmp );
    }
}

void SerialSet::writeOn( ostream& os, const char* tag ) const
{
    for( const_iterator citr = begin(); citr != end(); ++citr )
    {
        os << "\t" << tag << "\t0x" << hex << (*citr) << dec << pf_endl;
    }
}


Guild::Guild( ConfigElem& elem ) :
    _guildid( elem.remove_ushort( "GUILDID" ) ),
    _member_serials( elem, "MEMBER" ),
    _allyguild_serials( elem, "ALLYGUILD" ),
    _enemyguild_serials( elem, "ENEMYGUILD" ),
    _proplist(),
    _disbanded(false)
{
    _proplist.readProperties( elem );
}

Guild::Guild( unsigned long guildid ) :
    _guildid(guildid),
    _member_serials(),
    _allyguild_serials(),
    _enemyguild_serials(),
    _proplist(),
    _disbanded(false)
{
    if (_guildid >= nextguildid)
        nextguildid = _guildid+1;
}

void Guild::registerWithMembers()
{
    for( SerialSet::iterator itr = _member_serials.begin(); itr != _member_serials.end(); /* */)
    {
        unsigned long mserial = (*itr);
        SerialSet::iterator last_itr = itr;
        ++itr;

        Character* chr = system_find_mobile( mserial );
        if (chr != NULL)
        {
            chr->guild( this );
        }
        else
        {
            _member_serials.erase( last_itr );
        }
    }
}

void Guild::update_online_members()
{
// FIXME : All of guilds.cpp iterator's need rewritten. Turley found a much better method
// used in the party system.
// NOTE: stlport seems to not return save itr on erase, but with a list container the iterator should stay valid
	for( SerialSet::iterator itr = _member_serials.begin(); itr != _member_serials.end(); /* */)
    {
        unsigned long mserial = (*itr);
        SerialSet::iterator last_itr = itr;
        ++itr;

        Character* chr = system_find_mobile( mserial );
        if (chr != NULL)
        {
			if (chr->client)
			{
				send_move( chr->client, chr );
				send_remove_character_to_nearby_cansee( chr );
				send_create_mobile_to_nearby_cansee( chr );
			}
        }
        else
        {
            _member_serials.erase( last_itr );
        }
    }
}

void Guild::update_online_members_remove( Character* chr )
{
	if (chr->client)
	{
		send_move( chr->client, chr );
		send_remove_character_to_nearby_cansee( chr );
		send_create_mobile_to_nearby_cansee( chr );
	}
}

unsigned long Guild::guildid() const
{
    return _guildid;
}

bool Guild::disbanded() const
{
    return _disbanded;
}

void Guild::disband()
{
    _disbanded = true;
}

bool Guild::hasMembers() const
{
    return !_member_serials.empty();
}

bool Guild::hasAllies() const
{
    return !_allyguild_serials.empty();
}
bool Guild::hasAlly( const Guild* g2 ) const
{
    return _allyguild_serials.count( g2->guildid() ) != 0;
}

bool Guild::hasEnemies() const
{
    return !_enemyguild_serials.empty();
}
bool Guild::hasEnemy( const Guild* g2 ) const
{
    return _enemyguild_serials.count( g2->guildid() ) != 0;
}


void Guild::printOn( ostream& os ) const
{
    os << "Guild" << pf_endl
       << "{" << pf_endl
       << "\tGuildId\t" << _guildid << pf_endl;
    _member_serials.writeOn( os, "Member" );
    _allyguild_serials.writeOn( os, "AllyGuild" );
    _enemyguild_serials.writeOn( os, "EnemyGuild" );
    _proplist.printProperties( os );
    os << "}" << pf_endl
       << pf_endl;
}

void Guild::addMember( unsigned long serial )
{
    _member_serials.insert( serial );
}

void register_guilds()
{
    for( Guilds::iterator itr = guilds.begin(); itr != guilds.end(); ++itr )
    {
        Guild* guild = (*itr).second.get();
        guild->registerWithMembers();
    }
}

void read_guilds_dat()
{
    string guildsfile = config.world_data_path + "guilds.txt";

	if (!FileExists( guildsfile ))
        return;

    ConfigFile cf( guildsfile );
    ConfigSection sect_general( cf, "GENERAL", CST_UNIQUE );
    ConfigSection sect_guild( cf, "GUILD", CST_NORMAL );
    ConfigElem elem;
    while (cf.read( elem ))
    {
        if (sect_general.matches( elem ))
        {
            nextguildid = elem.remove_ulong( "NEXTGUILDID", 1 );
        }
        else if (sect_guild.matches( elem ))
        {
            Guild* guild = new Guild(elem);
            guilds[ guild->guildid() ].set( guild );
        }
    }

    register_guilds();
}

void write_guilds( ostream& os )
{
    os << "General" << pf_endl
       << "{" << pf_endl
       << "\tNextGuildId\t" << nextguildid << pf_endl
       << "}" << pf_endl
       << pf_endl;

    for( Guilds::const_iterator itr = guilds.begin(); itr != guilds.end(); ++itr )
    {
        const Guild* guild = (*itr).second.get();
        guild->printOn( os );
    }
}

Guild* FindOrCreateGuild( unsigned long guildid, unsigned long memberserial )
{
    Guilds::iterator itr = guilds.find( guildid );
    Guild* guild = NULL;
    if (itr != guilds.end())
        guild = (*itr).second.get();
    else
    {
        guild = new Guild( guildid );
        guilds[ guildid ].set( guild );
    }

    // this is called from Character::readCommonProperties, which assigns
    // chr->guild (so we don't do it here)
    guild->addMember( memberserial );

    return guild;
}
Guild* FindGuild( unsigned long guildid )
{
    Guilds::iterator itr = guilds.find( guildid );
    Guild* guild = NULL;
    
    if (itr != guilds.end())
        guild = (*itr).second.get();

    return guild;
}

BApplicObjType guild_type;


EGuildRefObjImp::EGuildRefObjImp( GuildRef gref ) : BApplicObj<GuildRef>(&guild_type,gref)
{
};

const char* EGuildRefObjImp::typeOf() const
{
    return "GuildRef";
}

BObjectImp* EGuildRefObjImp::copy() const
{
    return new EGuildRefObjImp( obj_ );
}

bool EGuildRefObjImp::isTrue() const
{
    return (!obj_->_disbanded);   
}

bool EGuildRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &guild_type)
		{
			const EGuildRefObjImp* guildref_imp =
					explicit_cast<const EGuildRefObjImp*,const BApplicObjBase*>(aob);

			return (guildref_imp->obj_->_guildid == obj_->_guildid);
		}
		else
			return false;
	}
	else
		return false;
}

BObjectImp* CreateGuildRefObjImp( Guild* guild )
{
    return new EGuildRefObjImp( ref_ptr<Guild>(guild) );
}

bool getGuildParam( Executor& exec, 
                    unsigned param,
                    Guild*& guild,
                    BError*& err )
{
    BApplicObjBase* aob = NULL;
    if (exec.hasParams( param+1 ))
        aob = exec.getApplicObjParam( param, &guild_type );

    if (aob == NULL)
    {
        err = new BError( "Invalid parameter type" );
        return false;
    }

    EGuildRefObjImp* gr = static_cast<EGuildRefObjImp*>(aob);
    guild = gr->value().get();
    if (guild->disbanded())
    {
        err = new BError( "Guild has disbanded" );
        return false;
    }
    return true;
}

BObjectRef EGuildRefObjImp::get_member_id( const int id ) //id test
{
    if (obj_->_disbanded)
        return BObjectRef( new BError( "Guild has disbanded" ) );
 
    ObjArray* arr;
    switch(id)
    {
    case MBR_MEMBERS:
        arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_member_serials.begin();
             itr != obj_->_member_serials.end();
             /* do this earlier */)
        {
            unsigned long mserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Character* chr = system_find_mobile( mserial );
            if (chr != NULL)
            {
                arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
            }
            else
            {
                obj_->_member_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );

    case MBR_ALLYGUILDS:
        arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_allyguild_serials.begin();
             itr != obj_->_allyguild_serials.end();
             /* do this earlier */)
        {
            unsigned long gserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Guild* guild = FindGuild( gserial );

            if (guild != NULL)
            {
                arr->addElement( new EGuildRefObjImp( ref_ptr<Guild>(guild) ) );
            }
            else
            {
                obj_->_allyguild_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );

    case MBR_ENEMYGUILDS:
        arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_enemyguild_serials.begin();
             itr != obj_->_enemyguild_serials.end();
             /* do this earlier */)
        {
            unsigned long gserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Guild* guild = FindGuild( gserial );

            if (guild != NULL)
            {
                arr->addElement( new EGuildRefObjImp( ref_ptr<Guild>(guild) ) );
            }
            else
            {
                obj_->_enemyguild_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );

    case MBR_GUILDID:
        return BObjectRef( new BLong( obj_->_guildid ) );
    default:
        return BObjectRef( UninitObject::create() );
    }
}
BObjectRef EGuildRefObjImp::get_member( const char* membername )
{
    if (obj_->_disbanded)
        return BObjectRef(new BError("Guild has disbanded"));
    
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
	/*
    if (stricmp( membername, "members") == 0)
    {
        ObjArray* arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_member_serials.begin(); itr != obj_->_member_serials.end(); )
             //do this earlier
        {
            unsigned long mserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Character* chr = system_find_mobile( mserial, SYSFIND_SEARCH_OFFLINE_MOBILES );
            if (chr != NULL)
            {
                arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
            }
            else
            {
                obj_->_member_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );
    }
    else if (stricmp( membername, "allyguilds") == 0)
    {
        ObjArray* arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_allyguild_serials.begin(); itr != obj_->_allyguild_serials.end(); )
             //do this earlier
        {
            unsigned long gserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Guild* guild = FindGuild( gserial );

            if (guild != NULL)
            {
                arr->addElement( new EGuildRefObjImp( ref_ptr<Guild>(guild) ) );
            }
            else
            {
                obj_->_allyguild_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );
    }
    else if (stricmp( membername, "enemyguilds") == 0)
    {
        ObjArray* arr = new ObjArray;
        for( SerialSet::iterator itr = obj_->_enemyguild_serials.begin();
             itr != obj_->_enemyguild_serials.end(); )
             //do this earlier
        {
            unsigned long gserial = (*itr);
            SerialSet::iterator last_itr = itr;
            ++itr;

            Guild* guild = FindGuild( gserial );

            if (guild != NULL)
            {
                arr->addElement( new EGuildRefObjImp( ref_ptr<Guild>(guild) ) );
            }
            else
            {
                obj_->_enemyguild_serials.erase( last_itr );
            }
        }
        return BObjectRef( arr );
    }
    else if (stricmp( membername, "guildid") == 0)
    {
        return BObjectRef( new BLong( obj_->_guildid ) );
    }
    else
        return BObjectRef( UninitObject::create() );
	*/
}

BObjectImp* EGuildRefObjImp::call_method_id( const int id, Executor& ex )
{
    if (obj_->_disbanded)
        return new BError( "Guild has disbanded" );

    Guild* enemyguild;
    Guild* allyguild;
    BError* err;
    Character* chr;
    switch(id)
    {
    case MTH_ISMEMBER:
        if (!ex.hasParams(1))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );

        return new BLong(obj_->_member_serials.count( chr->serial ));

    case MTH_ISALLYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            return new BLong( obj_->_allyguild_serials.count(allyguild->guildid()) );
        }
        else
        {
            return err;
        }

    case MTH_ISENEMYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            return new BLong( obj_->_enemyguild_serials.count(enemyguild->guildid()) );
        }
        else
        {
            return err;
        }
    
    case MTH_ADDMEMBER:
        if (!ex.hasParams( 1 ))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );
        if (chr->guildid())
            return new BError( "Character already belongs to a guild" );

        chr->guild( obj_.get() );
        obj_->_member_serials.insert( chr->serial );

		// MuadDib Added to update online members when status changes.
		obj_->update_online_members();

        return new BLong(1);

    case MTH_ADDALLYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            if (obj_->_enemyguild_serials.count(allyguild->guildid()))
                return new BError( "That is an enemy guild" );
            if (allyguild->guildid() == obj_->guildid())
                return new BError( "Passed self as new ally guild" );
            
            obj_->_allyguild_serials.insert( allyguild->guildid() );
            allyguild->_allyguild_serials.insert( obj_->_guildid );

			// MuadDib Added to update online members when status changes.
			obj_->update_online_members();
			allyguild->update_online_members();

            return new BLong(1);
        }
        else
        {
            return err;
        }

    case MTH_ADDENEMYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            if (obj_->_allyguild_serials.count(enemyguild->guildid()))
                return new BError( "That is an ally guild" );
            if (enemyguild->guildid() == obj_->guildid())
                return new BError( "Passed self as new enemy guild" );

            obj_->_enemyguild_serials.insert( enemyguild->guildid() );
            enemyguild->_enemyguild_serials.insert( obj_->_guildid );

			// MuadDib Added to update online members when status changes.
			obj_->update_online_members();
			enemyguild->update_online_members();

            return new BLong(1);
        }
        else
        {
            return err;
        }
    
    case MTH_REMOVEMEMBER:
        if (!ex.hasParams( 1 ))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );
        if (chr->guildid() != obj_->_guildid)
            return new BError( "Character does not belong to this guild" );

        chr->guild( NULL );
        obj_->_member_serials.erase( chr->serial );

		// MuadDib Added to update online members when status changes.
		obj_->update_online_members_remove( chr );

        return new BLong(1);

    case MTH_REMOVEALLYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            if (!obj_->_allyguild_serials.count(allyguild->guildid()))
                return new BError( "That is not an ally guild" );

            obj_->_allyguild_serials.erase( allyguild->guildid() );
            allyguild->_allyguild_serials.erase( obj_->_guildid );

			// MuadDib Added to update online members when status changes.
			obj_->update_online_members();
			allyguild->update_online_members();

            return new BLong(1);
        }
        else
        {
            return err;
        }

    case MTH_REMOVEENEMYGUILD:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            if (!obj_->_enemyguild_serials.count(enemyguild->guildid()))
                return new BError( "That is not an enemy guild" );

            obj_->_enemyguild_serials.erase( enemyguild->guildid() );
            enemyguild->_enemyguild_serials.erase( obj_->_guildid );

			// MuadDib Added to update online members when status changes.
			obj_->update_online_members();
			enemyguild->update_online_members();

            return new BLong(1);
        }
        else
        {
            return err;
        }
    default:
        bool changed = false;
        return CallPropertyListMethod_id( obj_->_proplist, id, ex, changed );
    }
}

BObjectImp* EGuildRefObjImp::call_method( const char* methodname, Executor& ex )
{
    if (obj_->_disbanded)
        return new BError("Guild has disbanded");

	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
	{
		bool changed = false;
        return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
    }
/*
    if (stricmp( methodname, "ismember" ) == 0)
    {
        Character* chr;
        if (!ex.hasParams(1))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );

        return new BLong(obj_->_member_serials.count( chr->serial ));
    }
    else if (stricmp( methodname, "isallyguild" ) == 0)
    {
        Guild* allyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            return new BLong( obj_->_allyguild_serials.count(allyguild->guildid()) );
        }
        else
        {
            return err;
        }
    }
    else if (stricmp( methodname, "isenemyguild" ) == 0)
    {
        Guild* enemyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            return new BLong( obj_->_enemyguild_serials.count(enemyguild->guildid()) );
        }
        else
        {
            return err;
        }
    }

    else if (stricmp( methodname, "addmember" ) == 0)
    {
        Character* chr;
        if (!ex.hasParams( 1 ))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );
        if (chr->guildid())
            return new BError( "Character already belongs to a guild" );

        chr->guild( obj_.get() );
        obj_->_member_serials.insert( chr->serial );
        
        return new BLong(1);
    }
    else if (stricmp( methodname, "addallyguild" ) == 0)
    {
        Guild* allyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            if (obj_->_enemyguild_serials.count(allyguild->guildid()))
                return new BError( "That is an enemy guild" );
            if (allyguild->guildid() == obj_->guildid())
                return new BError( "Passed self as new ally guild" );
            
            obj_->_allyguild_serials.insert( allyguild->guildid() );
            allyguild->_allyguild_serials.insert( obj_->_guildid );
            
            return new BLong(1);
        }
        else
        {
            return err;
        }
    }
    else if (stricmp( methodname, "addenemyguild" ) == 0)
    {
        Guild* enemyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            if (obj_->_allyguild_serials.count(enemyguild->guildid()))
                return new BError( "That is an ally guild" );
            if (enemyguild->guildid() == obj_->guildid())
                return new BError( "Passed self as new enemy guild" );

            obj_->_enemyguild_serials.insert( enemyguild->guildid() );
            enemyguild->_enemyguild_serials.insert( obj_->_guildid );
            
            return new BLong(1);
        }
        else
        {
            return err;
        }
    }

    else if (stricmp( methodname, "removemember" ) == 0)
    {
        Character* chr;
        if (!ex.hasParams( 1 ))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );
        if (chr->guildid() != obj_->_guildid)
            return new BError( "Character does not belong to this guild" );

        chr->guild( NULL );
        obj_->_member_serials.erase( chr->serial );
        
        return new BLong(1);
    }
    else if (stricmp( methodname, "removeallyguild" ) == 0)
    {
        Guild* allyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, allyguild, err ))
        {
            if (!obj_->_allyguild_serials.count(allyguild->guildid()))
                return new BError( "That is not an ally guild" );

            obj_->_allyguild_serials.erase( allyguild->guildid() );
            allyguild->_allyguild_serials.erase( obj_->_guildid );
            
            return new BLong(1);
        }
        else
        {
            return err;
        }
    }
    else if (stricmp( methodname, "removeenemyguild" ) == 0)
    {
        Guild* enemyguild;
        BError* err;

		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
        if (getGuildParam( ex, 0, enemyguild, err ))
        {
            if (!obj_->_enemyguild_serials.count(enemyguild->guildid()))
                return new BError( "That is not an enemy guild" );

            obj_->_enemyguild_serials.erase( enemyguild->guildid() );
            enemyguild->_enemyguild_serials.erase( obj_->_guildid );
            
            return new BLong(1);
        }
        else
        {
            return err;
        }
    }


    else
    {
        bool changed = false;
        return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
    }
	*/
}


/// uo.em functions:
///  ListGuilds(); // returns an array of Guild objects
BObjectImp* GuildExecutorModule::mf_ListGuilds()
{
    ObjArray* result = new ObjArray;
    for( Guilds::iterator itr = guilds.begin(); itr != guilds.end(); ++itr )
    {
        Guild* guild = (*itr).second.get();
        result->addElement( new EGuildRefObjImp( GuildRef(guild )) );
    }
    return result;
}

///  CreateGuild(); // returns a new Guild object
BObjectImp* GuildExecutorModule::mf_CreateGuild()
{
    Guild* guild = new Guild( nextguildid++ );
    
    guilds[ guild->guildid() ].set( guild );

    return new EGuildRefObjImp( ref_ptr<Guild>(guild) );
}

BObjectImp* GuildExecutorModule::mf_DestroyGuild()
{
    Guild* guild;
    BError* err;
    if (getGuildParam( exec, 0, guild, err ))
    {
        if (guild->hasMembers())
            return new BError( "Guild has members" );
        if (guild->hasAllies())
            return new BError( "Guild has allies" );
        if (guild->hasEnemies())
            return new BError( "Guild has enemies" );

        guild->disband();
        guilds.erase( guild->guildid() );
        return new BLong(1);
    }
    else
    {
        return err;
    }
}

///  FindGuild( guildid );
BObjectImp* GuildExecutorModule::mf_FindGuild()
{
    long guildid;
    if (getParam( 0, guildid ))
    {
        Guilds::iterator itr = guilds.find( guildid );
        if (itr != guilds.end())
            return new EGuildRefObjImp( ref_ptr<Guild>( (*itr).second.get() ) );
        else
            return new BError( "Guild not found" );
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

Guild* Character::guild() const
{
    return guild_;
}
void Character::guild( Guild* g )
{
    guild_ = g;
}
unsigned long Character::guildid() const
{
    return guild_ ? guild_->guildid() : 0;
}

bool AreAllies( Guild* g1, Guild* g2 )
{
    return (g1 == g2 ||
            g1->hasAlly(g2));
}
bool AreEnemies( Guild* g1, Guild* g2 )
{
    return g1->hasEnemy(g2);
}