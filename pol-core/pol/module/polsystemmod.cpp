/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2007/04/08 MuadDib:   Changed Realms() to get BObject IMP, and check for string
 *                         explicitly.
 * - 2009/11/30 Turley:    added MD5Encrypt(string)
 * - 2010/03/28 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within
 * decay_thread_shadow
 */


#include "polsystemmod.h"
#include <ctime>
#include <fstream>
#include <string>

#include "bscript/berror.h"
#include "bscript/bobject.h"
#include "bscript/dict.h"
#include "bscript/impstr.h"
#include "clib/clib.h"
#include "clib/clib_MD5.h"
#include "clib/fileutil.h"
#include "clib/rawtypes.h"
#include "clib/strutil.h"
#include "clib/threadhelp.h"
#include "plib/pkg.h"
#include "plib/systemstate.h"


#include "realms/realm.h"
#include "realms/realms.h"

#include "globals/settings.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"

#include "cmdlevel.h"
#include "core.h"
#include "decay.h"
#include "listenpt.h"
#include "packetscrobj.h"
#include "proplist.h"
#include "tooltips.h"
#include "uobject.h"
#include "uoexec.h"

#include <module_defs/polsys.h>

namespace Pol
{
namespace Core
{
void reload_configuration();
}  // namespace Core
namespace Module
{
using namespace Bscript;

BApplicObjType packageobjimp_type;

PackageObjImp::PackageObjImp( const PackagePtrHolder& other )
    : PackageObjImpBase( &packageobjimp_type, other )
{
}
const char* PackageObjImp::typeOf() const
{
  return "Package";
}
u8 PackageObjImp::typeOfInt() const
{
  return OTPackage;
}
BObjectImp* PackageObjImp::copy() const
{
  return new PackageObjImp( obj_ );
}
BObjectImp* PackageObjImp::call_polmethod( const char* /*methodname*/, Core::UOExecutor& /*ex*/ )
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
    const Plib::Package* pkg = value().Ptr();
    std::string filepath = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );
    return BObjectRef( new BLong( Clib::FileExists( filepath ) ) );
  }
  else if ( stricmp( membername, "dir" ) == 0 )
  {
    const Plib::Package* pkg = value().Ptr();
    return BObjectRef( new String( pkg->dir() ) );
  }
  else if ( stricmp( membername, "desc" ) == 0 )
  {
    return BObjectRef( new String( value()->desc() ) );
  }
  else
  {
    return BObjectRef( new BError( "Undefined member" ) );
  }
}

PolSystemExecutorModule::PolSystemExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<PolSystemExecutorModule, Core::PolModule>( exec )
{
}

BObjectImp* PolSystemExecutorModule::mf_IncRevision( /* uobject */ )
{
  Core::UObject* uobj;
  if ( getUObjectParam( 0, uobj ) )
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
    if ( cmdlevel_num >= static_cast<int>( Core::gamestate.cmdlevels.size() ) )
      cmdlevel_num = static_cast<int>( Core::gamestate.cmdlevels.size() - 1 );

    return new String( Core::gamestate.cmdlevels[cmdlevel_num].name );
  }
  else if ( getStringParam( 0, cmdlevel_alias ) )
  {
    Core::CmdLevel* cmdlevel = Core::FindCmdLevelByAlias( cmdlevel_alias->data() );
    if ( cmdlevel == nullptr )
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

  Core::CmdLevel* cmdlevel_search = Core::find_cmdlevel( cmdlvlname->data() );
  if ( cmdlevel_search == nullptr )
    return new BError( "Could not find a command level with that name." );

  return new BLong( cmdlevel_search->cmdlevel );
}

BObjectImp* PolSystemExecutorModule::mf_Packages()
{
  std::unique_ptr<ObjArray> arr( new ObjArray );
  for ( unsigned i = 0; i < Plib::systemstate.packages.size(); ++i )
  {
    PackageObjImp* imp = new PackageObjImp( PackagePtrHolder( Plib::systemstate.packages[i] ) );
    arr->addElement( imp );
  }
  return arr.release();
}

BObjectImp* PolSystemExecutorModule::mf_GetPackageByName()
{
  const String* pkgname;
  if ( !getStringParam( 0, pkgname ) )
    return new BError( "Invalid parameter type." );

  // pkgname->toLower();
  Plib::Package* pkg = Plib::find_package( pkgname->value() );
  if ( !pkg )
    return new BError( "No package found by that name." );
  else
    return new PackageObjImp( PackagePtrHolder( pkg ) );
}

BObjectImp* PolSystemExecutorModule::mf_ListTextCommands()
{
  std::unique_ptr<BDictionary> pkg_list( new BDictionary );

  int max_cmdlevel;
  if ( exec.numParams() < 1 || !getParam( 0, max_cmdlevel ) )
    max_cmdlevel = -1;

  // Sets up text commands not in a package.
  {
    auto cmd_lvl_list = Core::ListAllCommandsInPackage( nullptr, max_cmdlevel );
    if ( cmd_lvl_list->contents().size() > 0 )
      pkg_list->addMember( new String( "" ), cmd_lvl_list.release() );
  }

  // Sets up packaged text commands.
  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    auto cmd_lvl_list = Core::ListAllCommandsInPackage( pkg, max_cmdlevel );
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
  return new Double( static_cast<double>( Core::polclock_t_to_ms( Core::polclock() ) ) );
}

BObjectImp* PolSystemExecutorModule::mf_ListenPoints()
{
  return Core::ListenPoint::GetListenPoints();
}

BStruct* SetupRealmDetails( Realms::Realm* realm )
{
  std::unique_ptr<BStruct> details( new BStruct() );
  details->addMember( "width", new BLong( realm->width() ) );
  details->addMember( "height", new BLong( realm->height() ) );
  details->addMember( "season", new BLong( realm->season() ) );
  details->addMember( "mapid", new BLong( realm->getUOMapID() ) );
  details->addMember( "toplevel_item_count", new BLong( realm->toplevel_item_count() ) );
  details->addMember( "mobile_count", new BLong( realm->mobile_count() ) );
  details->addMember( "offline_mobs_count", new BLong( realm->offline_mobile_count() ) );
  details->addMember( "multi_count", new BLong( realm->multi_count() ) );

  return details.release();
}

BObjectImp* PolSystemExecutorModule::mf_Realms( /* realm_name:="" */ )
{
  const String* realm_name;
  //  getStringParam(0, realm_name);
  BObjectImp* imp = getParamImp( 0 );
  if ( imp->isa( BObjectImp::OTString ) )
  {
    realm_name = static_cast<const String*>( imp );
  }
  else
  {
    return new BError( std::string( "Parameter must be a String or empty, got " ) +
                       BObjectImp::typestr( imp->type() ) );
  }

  if ( realm_name->length() > 0 )
  {
    Realms::Realm* realm = Core::find_realm( realm_name->value() );
    if ( !realm )
      return new BError( "Realm not found." );
    else
      return SetupRealmDetails( realm );
  }
  else
  {
    BDictionary* dict = new BDictionary;
    std::vector<Realms::Realm*>::iterator itr;
    for ( itr = Core::gamestate.Realms.begin(); itr != Core::gamestate.Realms.end(); ++itr )
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
  if ( getObjtypeParam( 0, objtype ) )
  {
    const Items::ItemDesc& id = Items::find_itemdesc( objtype );
    if ( id.objtype == 0 && id.graphic == 0 )
      return new BError( "Itemdesc.cfg entry for objtype " + Clib::hexint( objtype ) +
                         " not found." );

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
  if ( exec.getParam( 0, type ) && exec.getParam( 1, size ) )
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

BObjectImp* PolSystemExecutorModule::mf_AddRealm( /*name,base*/ )
{
  const String* realm_name;
  const String* base;
  if ( !( getStringParam( 0, realm_name ) && getStringParam( 1, base ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Realms::Realm* baserealm = Core::find_realm( base->value() );
  if ( !baserealm )
    return new BError( "BaseRealm not found." );
  if ( baserealm->is_shadowrealm )
    return new BError( "BaseRealm is a ShadowRealm." );
  if ( Core::defined_realm( realm_name->value() ) )
    return new BError( "Realmname already defined." );
  Core::add_realm( realm_name->value(), baserealm );
  return new BLong( 1 );
}

BObjectImp* PolSystemExecutorModule::mf_DeleteRealm( /*name*/ )
{
  const String* realm_name;
  if ( !( getStringParam( 0, realm_name ) ) )
    return new BError( "Invalid parameter" );

  Realms::Realm* realm = Core::find_realm( realm_name->value() );

  if ( !realm )
    return new BError( "Realm not found." );
  if ( !realm->is_shadowrealm )
    return new BError( "Realm is not a ShadowRealm." );
  if ( realm->mobile_count() > 0 )
    return new BError( "Mobiles in Realm." );
  if ( realm->offline_mobile_count() > 0 )
    return new BError( "Offline characters in Realm" );
  if ( realm->toplevel_item_count() > 0 )
    return new BError( "Items in Realm." );
  if ( realm->multi_count() > 0 )
    return new BError( "Multis in Realm." );

  Core::remove_realm( realm_name->value() );
  return new BLong( 1 );
}

BObjectImp* PolSystemExecutorModule::mf_GetRealmDecay( /*name*/ )
{
  const String* realm_name;
  if ( !getStringParam( 0, realm_name ) )
  {
    return new BError( "Invalid parameter" );
  }
  Realms::Realm* realm = Core::find_realm( realm_name->value() );
  if ( !realm )
    return new BError( "Realm not found." );

  return new BBoolean( realm->has_decay );
}

BObjectImp* PolSystemExecutorModule::mf_SetRealmDecay( /*name,has_decay*/ )
{
  const String* realm_name;
  bool has_deacy;
  if ( !( getStringParam( 0, realm_name ) && getParam( 1, has_deacy ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Realms::Realm* realm = Core::find_realm( realm_name->value() );
  if ( !realm )
    return new BError( "Realm not found." );

  realm->has_decay = has_deacy;

  return new BLong( 1 );
}

BObjectImp* PolSystemExecutorModule::mf_MD5Encrypt( /*string*/ )
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

/**
 * Dumps the CProp profiling info into the log file
 *
 * @author Bodom
 */
BObjectImp* PolSystemExecutorModule::mf_LogCPropProfile()
{
  const std::string filepath = "log/cpprofile.log";
  std::ofstream ofs( filepath.c_str(), std::ios::out | std::ios::app );

  if ( !ofs.is_open() )
    return new BError( "Unable to open file: " + filepath );

  // Write the header
  auto t = std::time( nullptr );
  auto tm = Clib::localtime( t );
  ofs << std::string( 80, '=' ) << std::endl;
  ofs << "CProp profiling information dumped on " << std::asctime( &tm ) << std::endl;
  ofs << "the profiler is using an estimated amount of "
      << Core::CPropProfiler::instance().estimateSize() << " Bytes of memory." << std::endl;
  ofs << "the profiler is currently "
      << ( Plib::systemstate.config.profile_cprops ? "enabled" : "disabled" ) << "." << std::endl;
  ofs << std::endl;

  // Write the body
  Core::CPropProfiler::instance().dumpProfile( ofs );

  // Write the footer
  ofs << std::string( 80, '=' ) << std::endl;

  if ( ofs.fail() )
    return new BError( "Error during write." );

  ofs.close();
  return new BLong( 1 );
}
}  // namespace Module
}  // namespace Pol
