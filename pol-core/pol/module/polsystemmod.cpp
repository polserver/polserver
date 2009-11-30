/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2007/04/08 MuadDib:   Changed Realms() to get BObject IMP, and check for string
                      explicitly.
2009/11/30 Turley:    added MD5Encrypt(string)

Notes
=======

*/

#include "../../clib/stl_inc.h"
#include "../../clib/dirlist.h"
#include "../../clib/fileutil.h"
#include "../../clib/MD5.h"
#include "../../clib/strutil.h"
#include "../../clib/threadhelp.h"

#include "../../bscript/bobject.h"
#include "../../bscript/berror.h"
#include "../../bscript/dict.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/impstr.h"

#include "../../plib/pkg.h"
#include "../../plib/realm.h"

#include "../core.h"
#include "../cmdlevel.h"
#include "../item/itemdesc.h"
#include "../listenpt.h"
#include "polsystemmod.h"
#include "../polclock.h"
#include "../realms.h"
#include "../uoexhelp.h"
#include "../packetscrobj.h"
#include "../uobject.h"
#include "../tooltips.h"
#include "../ssopt.h"


class PackagePtrHolder
{
public:
	explicit PackagePtrHolder( Package* pkg ) : m_pPkg(pkg) {}
	Package* operator->() { return m_pPkg; }
	const Package* operator->() const { return m_pPkg; }

	Package* Ptr() { return m_pPkg; }
	const Package* Ptr() const { return m_pPkg; }
private:
	Package* m_pPkg;
};

BApplicObjType packageobjimp_type;
//typedef BApplicObj< ref_ptr<Package> > PackageObjImpBase;
typedef BApplicObj< PackagePtrHolder > PackageObjImpBase;
class PackageObjImp : public PackageObjImpBase
{
	typedef PackageObjImpBase base;
public:
	explicit PackageObjImp( const PackagePtrHolder& other );
	virtual const char* typeOf() const;
	virtual BObjectImp* copy() const;
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectRef get_member( const char* membername );
};
PackageObjImp::PackageObjImp( const PackagePtrHolder& other ) :
	  PackageObjImpBase(&packageobjimp_type, other )
{
}


template<>
TmplExecutorModule<PolSystemExecutorModule>::FunctionDef
  TmplExecutorModule<PolSystemExecutorModule>::function_table[] =
{
  { "IncRevision",			&PolSystemExecutorModule::mf_IncRevision },
  { "GetCmdLevelName",		&PolSystemExecutorModule::mf_GetCmdLevelName },
  { "GetCmdLevelNumber",		&PolSystemExecutorModule::mf_GetCmdLevelNumber },
  { "Packages",				&PolSystemExecutorModule::mf_Packages },
  { "GetPackageByName",		&PolSystemExecutorModule::mf_GetPackageByName },
  { "ListTextCommands",		&PolSystemExecutorModule::mf_ListTextCommands},
  { "Realms",					&PolSystemExecutorModule::mf_Realms },
  { "ReloadConfiguration",	&PolSystemExecutorModule::mf_ReloadConfiguration },
  { "ReadMillisecondClock",	&PolSystemExecutorModule::mf_ReadMillisecondClock },
  { "ListenPoints",			&PolSystemExecutorModule::mf_ListenPoints },
  { "SetSysTrayPopupText",	&PolSystemExecutorModule::mf_SetSysTrayPopupText },
  { "GetItemDescriptor",		&PolSystemExecutorModule::mf_GetItemDescriptor },
  { "CreatePacket",			&PolSystemExecutorModule::mf_CreatePacket },
  { "AddRealm",		        &PolSystemExecutorModule::mf_AddRealm },
  { "DeleteRealm",			&PolSystemExecutorModule::mf_DeleteRealm },
  { "MD5Encrypt",           &PolSystemExecutorModule::mf_MD5Encrypt }
};
template<>
int TmplExecutorModule<PolSystemExecutorModule>::function_table_size = arsize(function_table);

const char* PackageObjImp::typeOf() const
{
	return "Package";
}
BObjectImp* PackageObjImp::copy() const
{
	return new PackageObjImp( obj_ );
}
BObjectImp* PackageObjImp::call_method( const char* methodname, Executor& ex )
{
	return new BError( "undefined method" );
}
BObjectRef PackageObjImp::get_member( const char* membername )
{
	if ( stricmp(membername,"name") == 0 )
	{
		return BObjectRef(new String( value()->name() ));
	}
	else if ( stricmp(membername, "version") == 0 )
	{
		return BObjectRef(new String(value()->version()));
	}
	else if ( stricmp(membername, "supports_http") == 0 )
	{
		const Package* pkg = value().Ptr();
		return BObjectRef(new BLong(FileExists(pkg->dir()+"www")));
	}
	else if ( stricmp(membername, "npcdesc") == 0 )
	{
		Package* pkg = value().Ptr();
		string filepath = GetPackageCfgPath(pkg, "npcdesc.cfg");
		return BObjectRef(new BLong(FileExists(filepath)));
	}
	else
	{
		return BObjectRef( new BError( "Undefined member" ) );
	}
}

BObjectImp* PolSystemExecutorModule::mf_IncRevision( /* uobject */ )
{
    UObject* uobj;
	if (getUObjectParam( exec, 0, uobj ) )
    {
		uobj->increv();
		send_object_cache_to_inrange(uobj);
		return new BLong(1);
	}
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* PolSystemExecutorModule::mf_GetCmdLevelName()
{
	long cmdlevel_num;
	const String* cmdlevel_alias;

	if ( exec.numParams() != 1 )
		return new BError("Expected 1 parameter.");
	else if ( getParam(0, cmdlevel_num) )
	{
		if ( cmdlevel_num >= long(cmdlevels2.size()) )
			cmdlevel_num = cmdlevels2.size()-1;
		
		return new String(cmdlevels2[cmdlevel_num].name);
	}
	else if ( getStringParam(0, cmdlevel_alias) )
	{
		CmdLevel* cmdlevel = FindCmdLevelByAlias(cmdlevel_alias->data());
		if ( cmdlevel == NULL )
			return new BError("Could not find a command level with that alias.");
		else
			return new String(cmdlevel->name);
	}
	else
		return new BError("Invalid parameter type.");

}

BObjectImp* PolSystemExecutorModule::mf_GetCmdLevelNumber()
{
	const String* cmdlvlname;
	if ( !getStringParam(0, cmdlvlname) )
		return new BError("Invalid parameter type.");

	CmdLevel* cmdlevel_search = find_cmdlevel(cmdlvlname->data());
	if ( cmdlevel_search == NULL )
		return new BError("Could not find a command level with that name.");

	return new BLong(cmdlevel_search->cmdlevel);
}

BObjectImp* PolSystemExecutorModule::mf_Packages()
{
	ObjArray* arr = new ObjArray;
	for( unsigned i = 0; i < packages.size(); ++i )
	{
		PackageObjImp* imp = new PackageObjImp( PackagePtrHolder( packages[i] ) );
		arr->addElement( imp );
	}
	return arr;
}

BObjectImp* PolSystemExecutorModule::mf_GetPackageByName()
{
	const String* pkgname;
	if ( !getStringParam(0, pkgname) )
		return new BError("Invalid parameter type.");

	//pkgname->toLower();
	Package* pkg = find_package(pkgname->value());
	if ( !pkg )
		return new BError("No package found by that name.");
	else
		return new PackageObjImp(PackagePtrHolder(pkg));
}

BObjectImp* PolSystemExecutorModule::mf_ListTextCommands()
{
	BDictionary* pkg_list = new BDictionary;
	// Sets up text commands not in a package.
	{
		BDictionary* cmd_lvl_list = new BDictionary;
		for( unsigned num = 0; num < cmdlevels2.size(); ++num )
		{
			ObjArray* script_list = GetCommandsInPackage(NULL, num);
			if ( script_list == NULL ) 
				continue;
			else if ( script_list->ref_arr.size() > 0 )
				cmd_lvl_list->addMember(new BLong(num), script_list);
		}
		if ( cmd_lvl_list->contents().size() > 0 )
			pkg_list->addMember(new String(""), cmd_lvl_list);
	}
	//
	// Sets up packaged text commands.
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		BDictionary* cmd_lvl_list = new BDictionary;
		for( unsigned num = 0; num < cmdlevels2.size(); ++num )
		{
			ObjArray* script_list = GetCommandsInPackage(pkg, num);
			if ( script_list == NULL ) 
				continue;
			else if ( script_list->ref_arr.size() > 0 )
				cmd_lvl_list->addMember(new BLong(num), script_list);
		}
		if ( cmd_lvl_list->contents().size() > 0 )
			pkg_list->addMember(new String(pkg->name().c_str()), cmd_lvl_list);
	}
	return pkg_list;
}

void reload_configuration();

BObjectImp* PolSystemExecutorModule::mf_ReloadConfiguration()
{
	reload_configuration();

	return new BLong(1);
}

BObjectImp* PolSystemExecutorModule::mf_ReadMillisecondClock()
{
	return new BLong( polticks_t_to_ms( polclock() ) );
}

BObjectImp* PolSystemExecutorModule::mf_ListenPoints()
{
	return GetListenPoints();
}

BStruct* SetupRealmDetails(Realm* realm)
{
	BStruct* details = new BStruct();
	details->addMember("width", new BLong(realm->width()));
	details->addMember("height", new BLong(realm->height()));
	details->addMember("season", new BLong(realm->season()));
	details->addMember("mapid", new BLong(realm->getUOMapID()));
	details->addMember("toplevel_item_count", new BLong(realm->toplevel_item_count));
	details->addMember("mobile_count", new BLong(realm->mobile_count));

	return details;
}

BObjectImp* PolSystemExecutorModule::mf_Realms( /* realm_name:="" */ )
{
	const String* realm_name;
//	getStringParam(0, realm_name);
	BObjectImp* imp = getParamImp( 0 );
	if (imp->isa( BObjectImp::OTString ))
    {
		realm_name = static_cast<const String*>(imp);
    }
	else 
	{
		return new BError( string("Parameter must be a String or empty, got ") + BObjectImp::typestr( imp->type() ) );
	}

	if ( realm_name->length() > 0 )
	{
		Realm* realm = find_realm(realm_name->value());
		if( !realm ) 
			return new BError("Realm not found.");
		else
			return SetupRealmDetails(realm);
	}
	else
	{
		BDictionary* dict = new BDictionary;
		vector<Realm*>::iterator itr;
		for( itr = Realms->begin(); itr != Realms->end(); ++itr )
		{
			dict->addMember((*itr)->name().c_str(), SetupRealmDetails(*itr));
		}

		return dict;
	}
}

BObjectImp* PolSystemExecutorModule::mf_SetSysTrayPopupText()
{
#ifdef _WIN32
	const char* text = exec.paramAsString( 0 );
	
	CoreSetSysTrayToolTip( text, ToolTipPriorityScript );
#endif
	return new BLong( 1 );
}

BObjectImp* PolSystemExecutorModule::mf_GetItemDescriptor()
{
	unsigned short objtype;
	if (getObjtypeParam( exec, 0, objtype ))
	{
		const ItemDesc& id = find_itemdesc( objtype );
		if(id.objtype == 0 && id.graphic == 0)
			return new BError( "Itemdesc.cfg entry for objtype " + hexint(objtype) + " not found." );

		auto_ptr<BStruct> descriptor(new BStruct());
		
		id.PopulateStruct( descriptor.get() );
		
		return descriptor.release();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* PolSystemExecutorModule::mf_CreatePacket()
{
	long size;
	unsigned short type;
	if (exec.getParam( 0, type ) && 
		exec.getParam( 1, size ))
	{
		if(type > 0xFF)
			return new BError( "Packet type too high" );

		return new BPacket( static_cast<u8>(type), static_cast<signed short>(size) );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

void decay_thread_shadow( void* );
BObjectImp* PolSystemExecutorModule::mf_AddRealm(/*name,base*/)
{
	const String* realm_name;
	const String* base;
	if ( !(getStringParam(0, realm_name) &&
		getStringParam(1, base)))
	{
			return new BError("Invalid parameter");
    }
	Realm* baserealm = find_realm(base->value());
	if ( !baserealm ) 
		return new BError("BaseRealm not found.");
	if ( baserealm->is_shadowrealm )
		return new BError("BaseRealm is a ShadowRealm.");
	if ( defined_realm(realm_name->value()) )
		return new BError("Realmname already defined.");
	add_realm(realm_name->value(), baserealm);
	if (ssopt.decay_items)
	{
		ostringstream thname;
		thname << "Decay_" << realm_name->value();
		threadhelp::start_thread( decay_thread_shadow, thname.str().c_str(), (void*)(find_realm(realm_name->value())->shadowid) );
	}
	return new BLong(1);
}

BObjectImp* PolSystemExecutorModule::mf_DeleteRealm(/*name*/)
{
	const String* realm_name;
	if ( !(getStringParam(0, realm_name)))
		return new BError("Invalid parameter");
	Realm* realm = find_realm(realm_name->value());
	if ( !realm ) 
		return new BError("Realm not found.");
	if ( !realm->is_shadowrealm )
		return new BError("Realm is not a ShadowRealm.");
	if ( realm->mobile_count > 0 )
		return new BError("Mobiles in Realm.");
	if ( realm->toplevel_item_count > 0 )
		return new BError("Items in Realm.");
	remove_realm(realm_name->value());
	return new BLong(1);
}

BObjectImp* PolSystemExecutorModule::mf_MD5Encrypt(/*string*/)
{
    const String* string;
    if (!(getStringParam(0, string)))
		return new BError("Invalid parameter");
    if (string->length() < 1)
        return new BError("String is empty");
    std::string temp;
    if (!MD5_Encrypt(string->value(),temp))
        return new BError("Failed to encrypt");
    return new String(temp);
}
