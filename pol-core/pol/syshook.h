/** @file
 *
 * @par History
 * - 2009/10/22 Turley:    added OuchHook
 * - 2009/11/26 Turley:    Syshook CanDie(mobile)
 */


#ifndef SYSHOOK_H
#define SYSHOOK_H

#include "../bscript/bobject.h"

#include <memory>

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
class BApplicObjBase;
}
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
class Package;
}
namespace Core
{
class ExportScript;
class UObject;

class ExportedFunction
{
public:
  ExportedFunction( ExportScript*, unsigned PC );
  ~ExportedFunction();
  const std::string& scriptname() const;

  bool call( Bscript::BObjectImp* p0 );                           // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );  // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
             Bscript::BObjectImp* p2 );  // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2,
             Bscript::BObjectImp* p3 );  // throw()

  std::string call_string( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );  // throw()
  std::string call_string( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
                           Bscript::BObjectImp* p2 );  // throw()

  int call_long( Bscript::BObjectImp* p0 );                           // throw()
  int call_long( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );  // throw()
  Bscript::BObject call_object( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );
  Bscript::BObject call_object( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1,
                                Bscript::BObjectImp* p2 );

private:
  ExportScript* export_script;
  unsigned PC;
};

class SystemHooks
{
public:
  SystemHooks();
  void unload_system_hooks();

  std::unique_ptr<ExportedFunction> check_skill_hook;
  std::unique_ptr<ExportedFunction> open_spellbook_hook;
  std::unique_ptr<ExportedFunction> get_book_page_hook;
  std::unique_ptr<ExportedFunction> combat_advancement_hook;
  std::unique_ptr<ExportedFunction> parry_advancement_hook;
  std::unique_ptr<ExportedFunction> attack_hook;
  std::unique_ptr<ExportedFunction> pushthrough_hook;
  std::unique_ptr<ExportedFunction> speechmul_hook;
  std::unique_ptr<ExportedFunction> hitmiss_hook;
  std::unique_ptr<ExportedFunction> on_cast_hook;
  std::unique_ptr<ExportedFunction> can_decay;
  std::unique_ptr<ExportedFunction> ouch_hook;
  std::unique_ptr<ExportedFunction> can_die;
  std::unique_ptr<ExportedFunction> un_hide;
  std::unique_ptr<ExportedFunction> close_customhouse_hook;
  std::unique_ptr<ExportedFunction> warmode_change;
  std::unique_ptr<ExportedFunction> can_trade;
  std::unique_ptr<ExportedFunction> consume_ammunition_hook;

  Bscript::BObjectImp* call_script_method( const char* methodname, Bscript::Executor* ex,
                                           UObject* obj ) const;
  Bscript::BObjectImp* call_script_method( const char* methodname, Bscript::Executor* ex,
                                           Bscript::BApplicObjBase* obj ) const;

  bool get_method_hook( ExportScript* search_script, const char* methodname, Bscript::Executor* ex,
                        ExportScript** hook, unsigned int* PC ) const;
  std::unique_ptr<ExportScript> uobject_method_script;
  std::unique_ptr<ExportScript> item_method_script;
  std::unique_ptr<ExportScript> equipment_method_script;
  std::unique_ptr<ExportScript> lockable_method_script;
  std::unique_ptr<ExportScript> map_method_script;
  std::unique_ptr<ExportScript> multi_method_script;
  std::unique_ptr<ExportScript> armor_method_script;
  std::unique_ptr<ExportScript> weapon_method_script;
  std::unique_ptr<ExportScript> door_method_script;
  std::unique_ptr<ExportScript> container_method_script;
  std::unique_ptr<ExportScript> boat_method_script;
  std::unique_ptr<ExportScript> house_method_script;
  std::unique_ptr<ExportScript> spellbook_method_script;
  std::unique_ptr<ExportScript> corpse_method_script;
  std::unique_ptr<ExportScript> npc_method_script;
  std::unique_ptr<ExportScript> character_method_script;
  std::unique_ptr<ExportScript> client_method_script;
  std::unique_ptr<ExportScript> account_method_script;
  std::unique_ptr<ExportScript> party_method_script;
  std::unique_ptr<ExportScript> guild_method_script;
};

ExportedFunction* FindExportedFunction( Clib::ConfigElem& elem, const Plib::Package* pkg,
                                        const std::string& descriptor, unsigned nargs,
                                        bool complain_if_missing = true );
}
}
#endif
