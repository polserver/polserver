/*
History
=======
2009/10/22 Turley:    added OuchHook
2009/11/26 Turley:    Syshook CanDie(mobile)


Notes
=======

*/
#include "syshook.h"

#include "syshookscript.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "uoexec.h"

#include "../plib/pkg.h"

#include "../bscript/eprog.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"

#include <string>

/// SystemHookScript functions:
///	CheckSkill( who, skillid, difficulty, points )
namespace Pol {
  namespace Core {
    using namespace Bscript;

	ExportedFunction::ExportedFunction( ExportScript* shs, unsigned in_PC ) :
	  export_script( shs ),
	  PC( in_PC )
	{}
	ExportedFunction::~ExportedFunction()
	{
	  export_script = NULL;
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

	SystemHooks::SystemHooks() :
	  check_skill_hook( NULL ),
	  open_spellbook_hook( NULL ),
	  get_book_page_hook( NULL ),
	  combat_advancement_hook( NULL ),
	  parry_advancement_hook( NULL ),
	  attack_hook( NULL ),
	  pushthrough_hook( NULL ),
	  speechmul_hook( NULL ),
	  hitmiss_hook( NULL ),
	  on_cast_hook( NULL ),
	  can_decay( NULL ),
	  ouch_hook( NULL ),
	  can_die( NULL ),
	  un_hide( NULL ),
	  close_customhouse_hook( NULL ),
	  warmode_change( NULL ),
	  can_trade( NULL )
	{}


	std::vector<ExportScript*> export_scripts;
	SystemHooks system_hooks;

    void hook(ExportScript* shs, const std::string& hookname, const std::string& exfuncname)
	{
	  ExportedFunction** pphook = NULL;
	  unsigned nargs;
	  if ( hookname == "CheckSkill" )
	  {
		nargs = 4;
		pphook = &system_hooks.check_skill_hook;
	  }
	  else if ( hookname == "OpenSpellbook" )
	  {
		nargs = 1;
		pphook = &system_hooks.open_spellbook_hook;
	  }
	  else if ( hookname == "GetBookPage" )
	  {
		nargs = 2;
		pphook = &system_hooks.get_book_page_hook;
	  }
	  else if ( hookname == "CombatAdvancement" )
	  {
		nargs = 3;
		pphook = &system_hooks.combat_advancement_hook;
	  }
	  else if ( hookname == "ParryAdvancement" )
	  {
		nargs = 4;
		pphook = &system_hooks.parry_advancement_hook;
	  }
	  else if ( hookname == "Attack" )
	  {
		nargs = 2;
		pphook = &system_hooks.attack_hook;
	  }
	  else if ( hookname == "Pushthrough" )
	  {
		nargs = 2;
		pphook = &system_hooks.pushthrough_hook;
	  }
	  else if ( hookname == "SpeechMul" )
	  {
		nargs = 3;
		pphook = &system_hooks.speechmul_hook;
	  }
	  else if ( hookname == "HitMiss" )
	  {
		nargs = 2;
		pphook = &system_hooks.hitmiss_hook;
	  }
	  else if ( hookname == "OnCast" )
	  {
		nargs = 2;
		pphook = &system_hooks.on_cast_hook;
	  }
	  else if ( hookname == "CanDecay" )
	  {
		nargs = 1;
		pphook = &system_hooks.can_decay;
	  }
	  else if ( hookname == "Ouch" )
	  {
		nargs = 4;
		pphook = &system_hooks.ouch_hook;
	  }
	  else if ( hookname == "CanDie" )
	  {
		nargs = 1;
		pphook = &system_hooks.can_die;
	  }
	  else if ( hookname == "UnHide" )
	  {
		nargs = 1;
		pphook = &system_hooks.un_hide;
	  }
	  else if ( hookname == "CloseCustomHouse" )
	  {
		nargs = 2;
		pphook = &system_hooks.close_customhouse_hook;
	  }
	  else if ( hookname == "WarmodeChange" )
	  {
		nargs = 2;
		pphook = &system_hooks.warmode_change;
	  }
	  else if ( hookname == "CanTrade" )
	  {
		nargs = 3;
		pphook = &system_hooks.can_trade;
	  }
	  else
	  {
        INFO_PRINT << "Unknown SystemHook " << hookname << "\n";
		return;
	  }

	  if ( *pphook != NULL )
	  {
        INFO_PRINT << "SystemHook " << hookname << " multiply defined\n"
          << "  Already found in: " << system_hooks.check_skill_hook->scriptname() << "\n"
          << "  Also defined in:  " << shs->scriptname() << "\n";
		return;
	  }

	  unsigned PC;
	  if ( !shs->FindExportedFunction( exfuncname, nargs, PC ) )
	  {
        INFO_PRINT << "Exported Function " << exfuncname << " not found in " << shs->scriptname( ) << "\n";
		return;
	  }

	  *pphook = new ExportedFunction( shs, PC );
	}

	void load_system_hooks()
	{
	  /*
		  system_hooks.clear();
		  while (!export_scripts.empty())
		  {
		  delete export_scripts.back();
		  export_scripts.pop_back();
		  }
		  */
      for ( Plib::Packages::const_iterator citr = Plib::packages.begin( ); citr != Plib::packages.end( ); ++citr )
	  {
        Plib::Package* pkg = ( *citr );
		//string fname = pkg->dir() + "syshook.cfg";
        std::string fname = Plib::GetPackageCfgPath(pkg, "syshook.cfg");
		if ( Clib::FileExists( fname.c_str() ) )
		{
          Clib::ConfigFile cf( fname.c_str( ), "SystemHookScript" );
          Clib::ConfigElem elem;
		  while ( cf.read( elem ) )
		  {
			ExportScript* shs = new ExportScript( pkg, elem.rest() );
			if ( shs->Initialize() )
			{
			  export_scripts.push_back( shs );
              std::string hookname, exfuncname;
			  while ( elem.remove_first_prop( &hookname, &exfuncname ) )
			  {
				if ( exfuncname.empty() )
				  exfuncname = hookname;
				hook( shs, hookname, exfuncname );
			  }
			}
			else
			{
			  delete shs;
			}
		  }
		}
	  }
	}

	void unload_system_hooks()
	{
	  for ( unsigned i = 0; i < export_scripts.size(); ++i )
	  {
		ExportScript* ps = export_scripts[i];
		delete ps;
	  }
	  export_scripts.clear();
	  if ( system_hooks.attack_hook != NULL )
		delete system_hooks.attack_hook;
	  if ( system_hooks.check_skill_hook != NULL )
		delete system_hooks.check_skill_hook;
	  if ( system_hooks.combat_advancement_hook != NULL )
		delete system_hooks.combat_advancement_hook;
	  if ( system_hooks.get_book_page_hook != NULL )
		delete system_hooks.get_book_page_hook;
	  if ( system_hooks.hitmiss_hook != NULL )
		delete system_hooks.hitmiss_hook;
	  if ( system_hooks.open_spellbook_hook != NULL )
		delete system_hooks.open_spellbook_hook;
	  if ( system_hooks.parry_advancement_hook != NULL )
		delete system_hooks.parry_advancement_hook;
	  if ( system_hooks.pushthrough_hook != NULL )
		delete system_hooks.pushthrough_hook;
	  if ( system_hooks.speechmul_hook != NULL )
		delete system_hooks.speechmul_hook;
	  if ( system_hooks.on_cast_hook != NULL )
		delete system_hooks.on_cast_hook;
	  if ( system_hooks.can_decay != NULL )
		delete system_hooks.can_decay;
	  if ( system_hooks.ouch_hook != NULL )
		delete system_hooks.ouch_hook;
	  if ( system_hooks.can_die != NULL )
		delete system_hooks.can_die;
	  if ( system_hooks.un_hide != NULL )
		delete system_hooks.un_hide;
	  if ( system_hooks.close_customhouse_hook != NULL )
		delete system_hooks.close_customhouse_hook;
	  if ( system_hooks.warmode_change != NULL )
		delete system_hooks.warmode_change;
	  if ( system_hooks.can_trade != NULL )
		delete system_hooks.can_trade;
	}

	ExportScript* FindExportScript( const ScriptDef& sd )
	{
	  for ( unsigned i = 0; i < export_scripts.size(); ++i )
	  {
		ExportScript* ps = export_scripts[i];
		if ( ps->scriptname() == sd.name() )
		  return ps;
	  }

	  ExportScript* ps = new ExportScript( sd );
	  if ( ps->Initialize() )
	  {
		export_scripts.push_back( ps );
		return ps;
	  }
	  else
	  {
		delete ps;
		return NULL;
	  }
	}

	// descriptor is script:function (or :pkg:script:function, or ::script:function?)
    ExportedFunction* FindExportedFunction( Clib::ConfigElem& elem,
                                            const Plib::Package* pkg,
											const std::string& descriptor,
											unsigned nargs,
											bool complain_if_missing )
	{
	  // first, split up script and functionname
      auto colon_pos = descriptor.find_last_of(':');
	  if ( colon_pos == std::string::npos )
		elem.throw_error( "Expected ':' in exported function descriptor (" + descriptor + ")" );

      std::string scriptname = descriptor.substr(0, colon_pos);
      std::string functionname = descriptor.substr(colon_pos + 1, std::string::npos);

	  ScriptDef sd( scriptname, pkg, "" );

	  ExportScript* export_script = FindExportScript( sd );
	  if ( !export_script )
	  {
		if ( complain_if_missing )
		  elem.throw_error( "Export Script " + sd.name() + " not found" );
		else
		  return NULL;
	  }

	  unsigned PC;
	  if ( !export_script->FindExportedFunction( functionname, nargs, PC ) )
	  {
		if ( complain_if_missing )
		  elem.throw_error( "Exported Function " + functionname + " not found in " + export_script->scriptname() );
		else
		  return NULL;
	  }

	  return new ExportedFunction( export_script, PC );
	}
  }
}