/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2007/04/08 MuadDib:   Changed Realms() to get BObject IMP, and check for string
                      explicitly.
2009/11/30 Turley:    added MD5Encrypt(string)
2010/03/28 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within decay_thread_shadow

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
#include "../item/item.h"
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

namespace Pol {
  namespace Core {
    void decay_thread_shadow( void* );
    void reload_configuration( );
  }
  namespace Module {
    class PackagePtrHolder
    {
    public:
      explicit PackagePtrHolder( Plib::Package* pkg ) : m_pPkg( pkg ) {}
      Plib::Package* operator->( ) { return m_pPkg; }
      const Plib::Package* operator->( ) const { return m_pPkg; }

      Plib::Package* Ptr() { return m_pPkg; }
      const Plib::Package* Ptr() const { return m_pPkg; }
    private:
      Plib::Package* m_pPkg;
    };

    Bscript::BApplicObjType packageobjimp_type;
    //typedef BApplicObj< ref_ptr<Package> > PackageObjImpBase;
    typedef Bscript::BApplicObj< PackagePtrHolder > PackageObjImpBase;
    class PackageObjImp : public PackageObjImpBase
    {
      typedef PackageObjImpBase base;
    public:
      explicit PackageObjImp( const PackagePtrHolder& other );
      virtual const char* typeOf() const;
      virtual int typeOfInt() const;
      virtual Bscript::BObjectImp* copy() const;
      virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
      virtual Bscript::BObjectRef get_member( const char* membername );
    };
    PackageObjImp::PackageObjImp( const PackagePtrHolder& other ) :
      PackageObjImpBase( &packageobjimp_type, other )
    {}

  }
  namespace Bscript {
    using namespace Module;
    template<>
    TmplExecutorModule<PolSystemExecutorModule>::FunctionDef
      TmplExecutorModule<PolSystemExecutorModule>::function_table[] =
    {
      { "IncRevision", &PolSystemExecutorModule::mf_IncRevision },
      { "GetCmdLevelName", &PolSystemExecutorModule::mf_GetCmdLevelName },
      { "GetCmdLevelNumber", &PolSystemExecutorModule::mf_GetCmdLevelNumber },
      { "Packages", &PolSystemExecutorModule::mf_Packages },
      { "GetPackageByName", &PolSystemExecutorModule::mf_GetPackageByName },
      { "ListTextCommands", &PolSystemExecutorModule::mf_ListTextCommands },
      { "Realms", &PolSystemExecutorModule::mf_Realms },
      { "ReloadConfiguration", &PolSystemExecutorModule::mf_ReloadConfiguration },
      { "ReadMillisecondClock", &PolSystemExecutorModule::mf_ReadMillisecondClock },
      { "ListenPoints", &PolSystemExecutorModule::mf_ListenPoints },
      { "SetSysTrayPopupText", &PolSystemExecutorModule::mf_SetSysTrayPopupText },
      { "GetItemDescriptor", &PolSystemExecutorModule::mf_GetItemDescriptor },
      { "FormatItemDescription", &PolSystemExecutorModule::mf_FormatItemDescription },
      { "CreatePacket", &PolSystemExecutorModule::mf_CreatePacket },
      { "AddRealm", &PolSystemExecutorModule::mf_AddRealm },
      { "DeleteRealm", &PolSystemExecutorModule::mf_DeleteRealm },
      { "MD5Encrypt", &PolSystemExecutorModule::mf_MD5Encrypt }
    };
    template<>
    int TmplExecutorModule<PolSystemExecutorModule>::function_table_size = arsize( function_table );
  }
  namespace Module {
    using namespace Bscript;

	const char* PackageObjImp::typeOf() const
	{
	  return "Package";
	}
	int PackageObjImp::typeOfInt() const
	{
	  return OTPackage;
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
	  if ( stricmp( membername, "name" ) == 0 )
	  {
		return BObjectRef( new String( value()->name() ) );
	  }
	  else if ( stricmp( membername, "version" ) == 0 )
	  {
		return BObjectRef( new String( value()->version() ) );
	  }
	  else if ( stricmp( membername, "supports_http" ) == 0 )
	  {
		const Plib::Package* pkg = value().Ptr();
		return BObjectRef( new BLong( Clib::FileExists( pkg->dir() + "www" ) ) );
	  }
	  else if ( stricmp( membername, "npcdesc" ) == 0 )
	  {
        Plib::Package* pkg = value( ).Ptr( );
        string filepath = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );
		return BObjectRef( new BLong( Clib::FileExists( filepath ) ) );
	  }
	  else
	  {
		return BObjectRef( new BError( "Undefined member" ) );
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_IncRevision( /* uobject */ )
	{
	  Core::UObject* uobj;
	  if ( getUObjectParam( exec, 0, uobj ) )
	  {
		uobj->increv();
		send_object_cache_to_inrange( uobj );
		return new BLong( 1 );
	  }
	  else
	  {
		return new BError( "Invalid parameter type" );
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_GetCmdLevelName()
	{
	  int cmdlevel_num;
	  const String* cmdlevel_alias;

	  if ( exec.numParams() != 1 )
		return new BError( "Expected 1 parameter." );
	  else if ( getParam( 0, cmdlevel_num ) )
	  {
		if ( cmdlevel_num >= static_cast<int>( Core::cmdlevels2.size() ) )
		  cmdlevel_num = static_cast<int>( Core::cmdlevels2.size() - 1 );

        return new String( Core::cmdlevels2[cmdlevel_num].name );
	  }
	  else if ( getStringParam( 0, cmdlevel_alias ) )
	  {
        Core::CmdLevel* cmdlevel = Core::FindCmdLevelByAlias( cmdlevel_alias->data( ) );
		if ( cmdlevel == NULL )
		  return new BError( "Could not find a command level with that alias." );
		else
		  return new String( cmdlevel->name );
	  }
	  else
		return new BError( "Invalid parameter type." );

	}

	BObjectImp* PolSystemExecutorModule::mf_GetCmdLevelNumber()
	{
	  const String* cmdlvlname;
	  if ( !getStringParam( 0, cmdlvlname ) )
		return new BError( "Invalid parameter type." );

      Core::CmdLevel* cmdlevel_search = Core::find_cmdlevel( cmdlvlname->data( ) );
	  if ( cmdlevel_search == NULL )
		return new BError( "Could not find a command level with that name." );

	  return new BLong( cmdlevel_search->cmdlevel );
	}

	BObjectImp* PolSystemExecutorModule::mf_Packages()
	{
	  std::unique_ptr<ObjArray> arr( new ObjArray );
	  for ( unsigned i = 0; i < Plib::packages.size(); ++i )
	  {
		PackageObjImp* imp = new PackageObjImp( PackagePtrHolder( Plib::packages[i] ) );
		arr->addElement( imp );
	  }
	  return arr.release();
	}

	BObjectImp* PolSystemExecutorModule::mf_GetPackageByName()
	{
	  const String* pkgname;
	  if ( !getStringParam( 0, pkgname ) )
		return new BError( "Invalid parameter type." );

	  //pkgname->toLower();
      Plib::Package* pkg = Plib::find_package( pkgname->value( ) );
	  if ( !pkg )
		return new BError( "No package found by that name." );
	  else
		return new PackageObjImp( PackagePtrHolder( pkg ) );
	}

	BObjectImp* PolSystemExecutorModule::mf_ListTextCommands()
	{
	  std::unique_ptr<BDictionary> pkg_list( new BDictionary );
	  // Sets up text commands not in a package.
	  {
		std::unique_ptr<BDictionary> cmd_lvl_list( new BDictionary );
		for ( unsigned num = 0; num < Core::cmdlevels2.size(); ++num )
		{
		  ObjArray* script_list = Core::GetCommandsInPackage( NULL, num );
		  if ( script_list == NULL )
			continue;
		  else if ( !script_list->ref_arr.empty() )
			cmd_lvl_list->addMember( new BLong( num ), script_list );
		}
		if ( cmd_lvl_list->contents().size() > 0 )
		  pkg_list->addMember( new String( "" ), cmd_lvl_list.release() );
	  }
	  //
	  // Sets up packaged text commands.
      for ( Plib::Packages::iterator itr = Plib::packages.begin( ); itr != Plib::packages.end( ); ++itr )
	  {
        Plib::Package* pkg = ( *itr );
		std::unique_ptr<BDictionary> cmd_lvl_list( new BDictionary );
		for ( unsigned num = 0; num < Core::cmdlevels2.size(); ++num )
		{
          ObjArray* script_list = Core::GetCommandsInPackage( pkg, num );
		  if ( script_list == NULL )
			continue;
		  else if ( !script_list->ref_arr.empty() )
			cmd_lvl_list->addMember( new BLong( num ), script_list );
		}
		if ( cmd_lvl_list->contents().size() > 0 )
		  pkg_list->addMember( new String( pkg->name().c_str() ), cmd_lvl_list.release() );
	  }
	  return pkg_list.release();
	}

	BObjectImp* PolSystemExecutorModule::mf_ReloadConfiguration()
	{
	  Core::reload_configuration();

	  return new BLong( 1 );
	}

	BObjectImp* PolSystemExecutorModule::mf_ReadMillisecondClock()
	{
      return new BLong( Core::polticks_t_to_ms( Core::polclock( ) ) );
	}

	BObjectImp* PolSystemExecutorModule::mf_ListenPoints()
	{
      return Core::GetListenPoints( );
	}

	BStruct* SetupRealmDetails( Plib::Realm* realm )
	{
	  std::unique_ptr<BStruct> details( new BStruct() );
	  details->addMember( "width", new BLong( realm->width() ) );
	  details->addMember( "height", new BLong( realm->height() ) );
	  details->addMember( "season", new BLong( realm->season() ) );
	  details->addMember( "mapid", new BLong( realm->getUOMapID() ) );
	  details->addMember( "toplevel_item_count", new BLong( realm->toplevel_item_count ) );
	  details->addMember( "mobile_count", new BLong( realm->mobile_count ) );

	  return details.release();
	}

	BObjectImp* PolSystemExecutorModule::mf_Realms( /* realm_name:="" */ )
	{
	  const String* realm_name;
	  //	getStringParam(0, realm_name);
	  BObjectImp* imp = getParamImp( 0 );
	  if ( imp->isa( BObjectImp::OTString ) )
	  {
		realm_name = static_cast<const String*>( imp );
	  }
	  else
	  {
		return new BError( string( "Parameter must be a String or empty, got " ) + BObjectImp::typestr( imp->type() ) );
	  }

	  if ( realm_name->length() > 0 )
	  {
        Plib::Realm* realm = Core::find_realm( realm_name->value( ) );
		if ( !realm )
		  return new BError( "Realm not found." );
		else
		  return SetupRealmDetails( realm );
	  }
	  else
	  {
		BDictionary* dict = new BDictionary;
        vector<Plib::Realm*>::iterator itr;
        for ( itr = Core::Realms->begin( ); itr != Core::Realms->end( ); ++itr )
		{
		  dict->addMember( ( *itr )->name().c_str(), SetupRealmDetails( *itr ) );
		}

		return dict;
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_SetSysTrayPopupText()
	{
#ifdef _WIN32
	  const char* text = exec.paramAsString( 0 );

      Core::CoreSetSysTrayToolTip( text, Core::ToolTipPriorityScript );
#endif
	  return new BLong( 1 );
	}

	BObjectImp* PolSystemExecutorModule::mf_GetItemDescriptor()
	{
	  unsigned int objtype;
      if ( Core::getObjtypeParam( exec, 0, objtype ) )
	  {
		const Items::ItemDesc& id = Items::find_itemdesc( objtype );
		if ( id.objtype == 0 && id.graphic == 0 )
		  return new BError( "Itemdesc.cfg entry for objtype " + Clib::hexint( objtype ) + " not found." );

		std::unique_ptr<BStruct> descriptor( new BStruct() );

		id.PopulateStruct( descriptor.get() );

		return descriptor.release();
	  }
	  else
	  {
		return new BError( "Invalid parameter type" );
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_FormatItemDescription()
	{
	  const String* desc;
	  unsigned short amount;
	  const String* suffix;

	  if ( getStringParam( 0, desc ) && getParam( 1, amount ) && getStringParam( 2, suffix ) )
	  {
		return new String( Core::format_description( 0, desc->value(), amount, suffix->value() ) );
	  }
	  else
	  {
		return new BError( "Invalid parameter type" );
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_CreatePacket()
	{
	  int size;
	  unsigned short type;
	  if ( exec.getParam( 0, type ) &&
		   exec.getParam( 1, size ) )
	  {
		if ( type > 0xFF )
		  return new BError( "Packet type too high" );

		return new Core::BPacket( static_cast<u8>( type ), static_cast<signed short>( size ) );
	  }
	  else
	  {
		return new BError( "Invalid parameter type" );
	  }
	}

	BObjectImp* PolSystemExecutorModule::mf_AddRealm(/*name,base*/ )
	{
	  const String* realm_name;
	  const String* base;
	  if ( !( getStringParam( 0, realm_name ) &&
		getStringParam( 1, base ) ) )
	  {
		return new BError( "Invalid parameter" );
	  }
	  Plib::Realm* baserealm = Core::find_realm( base->value() );
	  if ( !baserealm )
		return new BError( "BaseRealm not found." );
	  if ( baserealm->is_shadowrealm )
		return new BError( "BaseRealm is a ShadowRealm." );
      if ( Core::defined_realm( realm_name->value( ) ) )
		return new BError( "Realmname already defined." );
      Core::add_realm( realm_name->value( ), baserealm );
      if ( Core::ssopt.decay_items )
	  {
		ostringstream thname;
		thname << "Decay_" << realm_name->value();
        threadhelp::start_thread( Core::decay_thread_shadow, thname.str( ).c_str( ), (void*)Core::find_realm( realm_name->value( ) ) );
	  }
	  return new BLong( 1 );
	}

	BObjectImp* PolSystemExecutorModule::mf_DeleteRealm(/*name*/ )
	{
	  const String* realm_name;
	  if ( !( getStringParam( 0, realm_name ) ) )
		return new BError( "Invalid parameter" );
      Plib::Realm* realm = Core::find_realm( realm_name->value( ) );
	  if ( !realm )
		return new BError( "Realm not found." );
	  if ( !realm->is_shadowrealm )
		return new BError( "Realm is not a ShadowRealm." );
	  if ( realm->mobile_count > 0 )
		return new BError( "Mobiles in Realm." );
	  if ( realm->toplevel_item_count > 0 )
		return new BError( "Items in Realm." );
      Core::remove_realm( realm_name->value( ) );
	  return new BLong( 1 );
	}

	BObjectImp* PolSystemExecutorModule::mf_MD5Encrypt(/*string*/ )
	{
	  const String* string;
	  if ( !( getStringParam( 0, string ) ) )
		return new BError( "Invalid parameter" );
	  if ( string->length() < 1 )
		return new BError( "String is empty" );
	  std::string temp;
	  if ( !Clib::MD5_Encrypt( string->value(), temp ) )
		return new BError( "Failed to encrypt" );
	  return new String( temp );
	}
  }
}