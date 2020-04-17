/** @file
 *
 * @par History
 * - 2009/10/22 Turley:    added OuchHook
 * - 2009/11/26 Turley:    Syshook CanDie(mobile)
 */

#include "syshook.h"

#include <stddef.h>
#include <string>
#include <tuple>

#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "accounts/account.h"
#include "accounts/acscrobj.h"
#include "globals/uvars.h"
#include "guildscrobj.h"
#include "module/guildmod.h"
#include "module/partymod.h"
#include "partyscrobj.h"
#include "polclass.h"
#include "scrdef.h"
#include "syshookscript.h"
#include "uobject.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;

ExportedFunction::ExportedFunction( ExportScript* shs, unsigned in_PC )
    : export_script( shs ), PC( in_PC )
{
}
ExportedFunction::~ExportedFunction()
{
  export_script = nullptr;
  PC = 0;
}
const std::string& ExportedFunction::scriptname() const
{
  return export_script->scriptname();
}

bool ExportedFunction::call( BObjectImp* p0 )
{
  return export_script->call( PC, p0 );
}
bool ExportedFunction::call( BObjectImp* p0, BObjectImp* p1 )
{
  return export_script->call( PC, p0, p1 );
}
bool ExportedFunction::call( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  return export_script->call( PC, p0, p1, p2 );
}
bool ExportedFunction::call( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2, BObjectImp* p3 )
{
  return export_script->call( PC, p0, p1, p2, p3 );
}

std::string ExportedFunction::call_string( BObjectImp* p0, BObjectImp* p1 )
{
  return export_script->call_string( PC, p0, p1 );
}
std::string ExportedFunction::call_string( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  return export_script->call_string( PC, p0, p1, p2 );
}

int ExportedFunction::call_long( BObjectImp* p0 )
{
  return export_script->call_long( PC, p0 );
}

int ExportedFunction::call_long( BObjectImp* p0, BObjectImp* p1 )
{
  return export_script->call_long( PC, p0, p1 );
}

BObject ExportedFunction::call_object( BObjectImp* p0, BObjectImp* p1 )
{
  return export_script->call_object( PC, p0, p1 );
}

BObject ExportedFunction::call_object( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
  return export_script->call_object( PC, p0, p1, p2 );
}

SystemHooks::SystemHooks()
    : check_skill_hook( nullptr ),
      open_spellbook_hook( nullptr ),
      get_book_page_hook( nullptr ),
      combat_advancement_hook( nullptr ),
      parry_advancement_hook( nullptr ),
      attack_hook( nullptr ),
      pushthrough_hook( nullptr ),
      speechmul_hook( nullptr ),
      hitmiss_hook( nullptr ),
      on_cast_hook( nullptr ),
      can_decay( nullptr ),
      ouch_hook( nullptr ),
      can_die( nullptr ),
      un_hide( nullptr ),
      close_customhouse_hook( nullptr ),
      warmode_change( nullptr ),
      can_trade( nullptr ),
      consume_ammunition_hook( nullptr ),
      uobject_method_script( nullptr ),
      item_method_script( nullptr ),
      equipment_method_script( nullptr ),
      lockable_method_script( nullptr ),
      map_method_script( nullptr ),
      multi_method_script( nullptr ),
      armor_method_script( nullptr ),
      weapon_method_script( nullptr ),
      door_method_script( nullptr ),
      container_method_script( nullptr ),
      boat_method_script( nullptr ),
      house_method_script( nullptr ),
      spellbook_method_script( nullptr ),
      corpse_method_script( nullptr ),
      npc_method_script( nullptr ),
      character_method_script( nullptr ),
      client_method_script( nullptr ),
      account_method_script( nullptr ),
      party_method_script( nullptr ),
      guild_method_script( nullptr )
{
}

void hook( ExportScript* shs, const std::string& hookname, const std::string& exfuncname )
{
  std::unique_ptr<ExportedFunction>* pphook = nullptr;
  unsigned nargs;
  if ( hookname == "CheckSkill" )
  {
    nargs = 4;
    pphook = &gamestate.system_hooks.check_skill_hook;
  }
  else if ( hookname == "OpenSpellbook" )
  {
    nargs = 1;
    pphook = &gamestate.system_hooks.open_spellbook_hook;
  }
  else if ( hookname == "GetBookPage" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.get_book_page_hook;
  }
  else if ( hookname == "CombatAdvancement" )
  {
    nargs = 3;
    pphook = &gamestate.system_hooks.combat_advancement_hook;
  }
  else if ( hookname == "ParryAdvancement" )
  {
    nargs = 4;
    pphook = &gamestate.system_hooks.parry_advancement_hook;
  }
  else if ( hookname == "Attack" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.attack_hook;
  }
  else if ( hookname == "Pushthrough" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.pushthrough_hook;
  }
  else if ( hookname == "SpeechMul" )
  {
    nargs = 3;
    pphook = &gamestate.system_hooks.speechmul_hook;
  }
  else if ( hookname == "HitMiss" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.hitmiss_hook;
  }
  else if ( hookname == "OnCast" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.on_cast_hook;
  }
  else if ( hookname == "CanDecay" )
  {
    nargs = 1;
    pphook = &gamestate.system_hooks.can_decay;
  }
  else if ( hookname == "Ouch" )
  {
    nargs = 4;
    pphook = &gamestate.system_hooks.ouch_hook;
  }
  else if ( hookname == "CanDie" )
  {
    nargs = 1;
    pphook = &gamestate.system_hooks.can_die;
  }
  else if ( hookname == "UnHide" )
  {
    nargs = 1;
    pphook = &gamestate.system_hooks.un_hide;
  }
  else if ( hookname == "CloseCustomHouse" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.close_customhouse_hook;
  }
  else if ( hookname == "WarmodeChange" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.warmode_change;
  }
  else if ( hookname == "CanTrade" )
  {
    nargs = 3;
    pphook = &gamestate.system_hooks.can_trade;
  }
  else if ( hookname == "ConsumeAmmunition" )
  {
    nargs = 2;
    pphook = &gamestate.system_hooks.consume_ammunition_hook;
  }
  else
  {
    INFO_PRINT << "Unknown SystemHook " << hookname << "\n";
    return;
  }

  if ( *pphook != nullptr )
  {
    INFO_PRINT << "SystemHook " << hookname << " multiply defined\n"
               << "  Already found in: " << gamestate.system_hooks.check_skill_hook->scriptname()
               << "\n"
               << "  Also defined in:  " << shs->scriptname() << "\n";
    return;
  }

  unsigned PC;
  if ( !shs->FindExportedFunction( exfuncname, nargs, PC ) )
  {
    INFO_PRINT << "Exported Function " << exfuncname << " not found in " << shs->scriptname()
               << "\n";
    return;
  }

  *pphook = std::make_unique<ExportedFunction>( shs, PC );
}

namespace
{
void setMethod( std::unique_ptr<ExportScript>* script, Plib::Package* pkg,
                const std::string& scriptname )
{
  auto shs = std::make_unique<ExportScript>( pkg, scriptname );
  if ( shs->Initialize() )
    script->swap( shs );
}
}  // namespace

void load_system_hooks()
{
  for ( Plib::Packages::const_iterator citr = Plib::systemstate.packages.begin();
        citr != Plib::systemstate.packages.end(); ++citr )
  {
    Plib::Package* pkg = ( *citr );
    std::string fname = Plib::GetPackageCfgPath( pkg, "syshook.cfg" );
    if ( Clib::FileExists( fname.c_str() ) )
    {
      Clib::ConfigFile cf( fname.c_str(), "SystemHookScript SystemMethod" );
      Clib::ConfigElem elem;
      while ( cf.read( elem ) )
      {
        if ( !stricmp( elem.type(), "SystemHookScript" ) )
        {
          auto shs = std::make_unique<ExportScript>( pkg, elem.rest() );
          if ( shs->Initialize() )
          {
            std::string hookname, exfuncname;
            while ( elem.remove_first_prop( &hookname, &exfuncname ) )
            {
              if ( exfuncname.empty() )
                exfuncname = hookname;
              hook( shs.get(), hookname, exfuncname );
            }
            gamestate.export_scripts.push_back( std::move( shs ) );
          }
        }
        else  // SystemMethod
        {
          std::string hookclass, script;

          while ( elem.remove_first_prop( &hookclass, &script ) )
          {
            Clib::mklowerASCII( hookclass );
            if ( !hookclass.compare( "uobject" ) )
              setMethod( &gamestate.system_hooks.uobject_method_script, pkg, script );
            else if ( !hookclass.compare( "item" ) )
              setMethod( &gamestate.system_hooks.item_method_script, pkg, script );
            else if ( !hookclass.compare( "equipment" ) )
              setMethod( &gamestate.system_hooks.equipment_method_script, pkg, script );
            else if ( !hookclass.compare( "lockable" ) )
              setMethod( &gamestate.system_hooks.lockable_method_script, pkg, script );
            else if ( !hookclass.compare( "map" ) )
              setMethod( &gamestate.system_hooks.map_method_script, pkg, script );
            else if ( !hookclass.compare( "multi" ) )
              setMethod( &gamestate.system_hooks.multi_method_script, pkg, script );
            else if ( !hookclass.compare( "armor" ) )
              setMethod( &gamestate.system_hooks.armor_method_script, pkg, script );
            else if ( !hookclass.compare( "weapon" ) )
              setMethod( &gamestate.system_hooks.weapon_method_script, pkg, script );
            else if ( !hookclass.compare( "door" ) )
              setMethod( &gamestate.system_hooks.door_method_script, pkg, script );
            else if ( !hookclass.compare( "container" ) )
              setMethod( &gamestate.system_hooks.container_method_script, pkg, script );
            else if ( !hookclass.compare( "boat" ) )
              setMethod( &gamestate.system_hooks.boat_method_script, pkg, script );
            else if ( !hookclass.compare( "house" ) )
              setMethod( &gamestate.system_hooks.house_method_script, pkg, script );
            else if ( !hookclass.compare( "spellbook" ) )
              setMethod( &gamestate.system_hooks.spellbook_method_script, pkg, script );
            else if ( !hookclass.compare( "corpse" ) )
              setMethod( &gamestate.system_hooks.corpse_method_script, pkg, script );
            else if ( !hookclass.compare( "npc" ) )
              setMethod( &gamestate.system_hooks.npc_method_script, pkg, script );
            else if ( !hookclass.compare( "character" ) )
              setMethod( &gamestate.system_hooks.character_method_script, pkg, script );
            else if ( !hookclass.compare( "client" ) )
              setMethod( &gamestate.system_hooks.client_method_script, pkg, script );
            else if ( !hookclass.compare( "account" ) )
              setMethod( &gamestate.system_hooks.account_method_script, pkg, script );
            else if ( !hookclass.compare( "party" ) )
              setMethod( &gamestate.system_hooks.party_method_script, pkg, script );
            else if ( !hookclass.compare( "guild" ) )
              setMethod( &gamestate.system_hooks.guild_method_script, pkg, script );
            else
              POLLOG_INFO << "Unknown class used for method hook: " << hookclass << "\n";
          }
        }
      }
    }
  }
}

BObjectImp* SystemHooks::call_script_method( const char* methodname, Executor* ex,
                                             UObject* obj ) const
{
  ExportScript* hook;
  unsigned PC;
  if ( obj->get_method_hook( methodname, ex, &hook, &PC ) )
    return hook->call( PC, obj->make_ref(), ex->fparams );
  return nullptr;
}

Bscript::BObjectImp* SystemHooks::call_script_method( const char* methodname, Bscript::Executor* ex,
                                                      Bscript::BApplicObjBase* obj ) const
{
  ExportScript* script( nullptr );
  if ( obj->object_type() == &Accounts::accountobjimp_type )
    script = account_method_script.get();
  else if ( obj->object_type() == &Module::eclientrefobjimp_type )
    script = client_method_script.get();
  else if ( obj->object_type() == &Module::party_type )
    script = party_method_script.get();
  else if ( obj->object_type() == &Module::guild_type )
    script = guild_method_script.get();

  if ( script != nullptr )
  {
    unsigned PC;
    ExportScript* hook;
    if ( get_method_hook( script, methodname, ex, &hook, &PC ) )
      return hook->call( PC, obj, ex->fparams );
  }
  return nullptr;
}

bool SystemHooks::get_method_hook( ExportScript* search_script, const char* methodname,
                                   Bscript::Executor* ex, ExportScript** hook,
                                   unsigned int* PC ) const
{
  if ( search_script && search_script->FindExportedFunction(
                            methodname, static_cast<unsigned int>( ex->numParams() + 1 ), *PC ) )
  {
    *hook = search_script;
    return true;
  }
  return false;
}

void SystemHooks::unload_system_hooks()
{
  gamestate.export_scripts.clear();
  attack_hook.reset( nullptr );
  check_skill_hook.reset( nullptr );
  combat_advancement_hook.reset( nullptr );
  get_book_page_hook.reset( nullptr );
  hitmiss_hook.reset( nullptr );
  open_spellbook_hook.reset( nullptr );
  parry_advancement_hook.reset( nullptr );
  pushthrough_hook.reset( nullptr );
  speechmul_hook.reset( nullptr );
  on_cast_hook.reset( nullptr );
  can_decay.reset( nullptr );
  ouch_hook.reset( nullptr );
  can_die.reset( nullptr );
  un_hide.reset( nullptr );
  close_customhouse_hook.reset( nullptr );
  warmode_change.reset( nullptr );
  can_trade.reset( nullptr );
  consume_ammunition_hook.reset( nullptr );

  uobject_method_script.reset( nullptr );
  item_method_script.reset( nullptr );
  equipment_method_script.reset( nullptr );
  lockable_method_script.reset( nullptr );
  map_method_script.reset( nullptr );
  multi_method_script.reset( nullptr );
  armor_method_script.reset( nullptr );
  weapon_method_script.reset( nullptr );
  door_method_script.reset( nullptr );
  container_method_script.reset( nullptr );
  boat_method_script.reset( nullptr );
  house_method_script.reset( nullptr );
  spellbook_method_script.reset( nullptr );
  corpse_method_script.reset( nullptr );
  npc_method_script.reset( nullptr );
  character_method_script.reset( nullptr );
  client_method_script.reset( nullptr );
  account_method_script.reset( nullptr );
  party_method_script.reset( nullptr );
  guild_method_script.reset( nullptr );
}

ExportScript* FindExportScript( const ScriptDef& sd )
{
  for ( const auto& ps : gamestate.export_scripts )
  {
    if ( ps->scriptname() == sd.name() )
      return ps.get();
  }

  auto ps = std::make_unique<ExportScript>( sd );
  if ( ps->Initialize() )
  {
    gamestate.export_scripts.push_back( std::move( ps ) );
    return gamestate.export_scripts.back().get();
  }
  return nullptr;
}

// descriptor is script:function (or :pkg:script:function, or ::script:function?)
ExportedFunction* FindExportedFunction( Clib::ConfigElem& elem, const Plib::Package* pkg,
                                        const std::string& descriptor, unsigned nargs,
                                        bool complain_if_missing )
{
  // first, split up script and functionname
  auto colon_pos = descriptor.find_last_of( ':' );
  if ( colon_pos == std::string::npos )
    elem.throw_error( "Expected ':' in exported function descriptor (" + descriptor + ")" );

  std::string scriptname = descriptor.substr( 0, colon_pos );
  std::string functionname = descriptor.substr( colon_pos + 1, std::string::npos );

  ScriptDef sd( scriptname, pkg, "" );

  ExportScript* export_script = FindExportScript( sd );
  if ( !export_script )
  {
    if ( complain_if_missing )
      elem.throw_error( "Export Script " + sd.name() + " not found" );
    else
      return nullptr;
  }

  unsigned PC;
  if ( !export_script->FindExportedFunction( functionname, nargs, PC ) )
  {
    if ( complain_if_missing )
      elem.throw_error( "Exported Function " + functionname + " not found in " +
                        export_script->scriptname() );
    else
      return nullptr;
  }

  return new ExportedFunction( export_script, PC );
}
}  // namespace Core
}  // namespace Pol
