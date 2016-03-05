/** @file
 *
 * @par History
 * - 2009/10/22 Turley:    added OuchHook
 * - 2009/11/26 Turley:    Syshook CanDie(mobile)
 */


#ifndef SYSHOOK_H
#define SYSHOOK_H

#include "uoexec.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
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

class ExportedFunction
{
public:
  ExportedFunction( ExportScript*, unsigned PC );
  ~ExportedFunction();
  const std::string& scriptname() const;

  bool call( Bscript::BObjectImp* p0 ); // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 ); // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2 ); // throw()
  bool call( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2, Bscript::BObjectImp* p3 ); // throw()

  std::string call_string( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 ); // throw()
  std::string call_string( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2 ); // throw()

  int call_long( Bscript::BObjectImp* p0 ); // throw()
  int call_long( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 ); // throw()
  Bscript::BObject call_object( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1 );
  Bscript::BObject call_object( Bscript::BObjectImp* p0, Bscript::BObjectImp* p1, Bscript::BObjectImp* p2 );

private:
  ExportScript* export_script;
  unsigned PC;
};

class SystemHooks
{
public:
  SystemHooks();
  void unload_system_hooks();

  ExportedFunction* check_skill_hook;
  ExportedFunction* open_spellbook_hook;
  ExportedFunction* get_book_page_hook;
  ExportedFunction* combat_advancement_hook;
  ExportedFunction* parry_advancement_hook;
  ExportedFunction* attack_hook;
  ExportedFunction* pushthrough_hook;
  ExportedFunction* speechmul_hook;
  ExportedFunction* hitmiss_hook;
  ExportedFunction* on_cast_hook;
  ExportedFunction* can_decay;
  ExportedFunction* ouch_hook;
  ExportedFunction* can_die;
  ExportedFunction* un_hide;
  ExportedFunction* close_customhouse_hook;
  ExportedFunction* warmode_change;
  ExportedFunction* can_trade;
};

ExportedFunction* FindExportedFunction( Clib::ConfigElem& elem,
                                        const Plib::Package* pkg,
                                        const std::string& descriptor,
                                        unsigned nargs,
                                        bool complain_if_missing = true );

}
}
#endif
