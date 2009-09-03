/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"

#include "bscript/eprog.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"

#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "syshook.h"
#include "syshookscript.h"
#include "plib/pkg.h"
#include "uoexec.h"

/// SystemHookScript functions:
///	CheckSkill( who, skillid, difficulty, points )


ExportedFunction::ExportedFunction( ExportScript* shs, unsigned in_PC ) :
	export_script( shs ),
	PC(in_PC)
{
}
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
	return export_script->call_string( PC, p0,p1 );
}
std::string ExportedFunction::call_string( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 )
{
	return export_script->call_string( PC, p0,p1,p2 );
}

long ExportedFunction::call_long( BObjectImp* p0 )
{
	return export_script->call_long( PC, p0 );
}

long ExportedFunction::call_long( BObjectImp* p0, BObjectImp* p1 )
{
	return export_script->call_long( PC, p0, p1 );
}

BObject ExportedFunction::call_object(BObjectImp* p0, BObjectImp* p1)
{
	return export_script->call_object( PC, p0,p1 );

}

BObject ExportedFunction::call_object(BObjectImp* p0, BObjectImp* p1,BObjectImp* p2)
{
	return export_script->call_object( PC, p0,p1,p2 );

}

SystemHooks::SystemHooks() :
	check_skill_hook(NULL),
	open_spellbook_hook(NULL),
	get_book_page_hook(NULL),
	combat_advancement_hook(NULL),
	parry_advancement_hook(NULL),
	attack_hook(NULL),
	pushthrough_hook(NULL),
	speechmul_hook(NULL),
	hitmiss_hook(NULL),
	on_cast_hook(NULL)
{
}


std::vector<ExportScript*> export_scripts;
SystemHooks system_hooks;

void hook( ExportScript* shs, const string& hookname, const string& exfuncname )
{
	ExportedFunction** pphook = NULL;
	unsigned nargs;
	if (hookname == "CheckSkill")
	{
		nargs = 4;
		pphook = &system_hooks.check_skill_hook;
	}
	else if (hookname == "OpenSpellbook")
	{
		nargs = 1;
		pphook = &system_hooks.open_spellbook_hook;
	}
	else if (hookname == "GetBookPage")
	{
		nargs = 2;
		pphook = &system_hooks.get_book_page_hook;
	}
	else if (hookname == "CombatAdvancement")
	{
		nargs = 3;
		pphook = &system_hooks.combat_advancement_hook;
	}
	else if (hookname == "ParryAdvancement")
	{
		nargs = 4;
		pphook = &system_hooks.parry_advancement_hook;
	}
	else if (hookname == "Attack")
	{
		nargs = 2;
		pphook = &system_hooks.attack_hook;
	}
	else if (hookname == "Pushthrough")
	{
		nargs = 2;
		pphook = &system_hooks.pushthrough_hook;
	}
	else if (hookname == "SpeechMul")
	{
		nargs = 3;
		pphook = &system_hooks.speechmul_hook;
	}
	else if (hookname == "HitMiss")
	{
		nargs = 2;
		pphook = &system_hooks.hitmiss_hook;
	}
	else if (hookname == "OnCast")
	{
		nargs = 2;
		pphook = &system_hooks.on_cast_hook;
	}
	else
	{
		cout << "Unknown SystemHook " << hookname << endl;
		return;
	}

	if (*pphook != NULL)
	{
		cout << "SystemHook " << hookname << " multiply defined" << endl;
		cout << "  Already found in: " << system_hooks.check_skill_hook->scriptname() << endl;
		cout << "  Also defined in:  " << shs->scriptname() << endl;
		return;
	}

	unsigned PC;
	if (!shs->FindExportedFunction( exfuncname, nargs, PC ))
	{
		cout << "Exported Function " << exfuncname << " not found in " << shs->scriptname() << endl;
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
	for( Packages::const_iterator citr = packages.begin(); citr != packages.end(); ++citr )
	{
		Package* pkg = (*citr);
		//string fname = pkg->dir() + "syshook.cfg";
		string fname = GetPackageCfgPath(pkg, "syshook.cfg");
		if (FileExists( fname.c_str() ))
		{
			ConfigFile cf( fname.c_str(), "SystemHookScript" );
			ConfigElem elem;
			while (cf.read( elem ))
			{
				ExportScript* shs = new ExportScript( pkg, elem.rest() );
				if (shs->Initialize())
				{
					export_scripts.push_back( shs );
					string hookname, exfuncname;
					while (elem.remove_first_prop( &hookname, &exfuncname ))
					{
						if (exfuncname.empty())
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
	for( unsigned i = 0; i < export_scripts.size(); ++i )
	{
		ExportScript* ps = export_scripts[i];
		delete ps;
	}
	export_scripts.clear();
	if (system_hooks.attack_hook != NULL)
		delete system_hooks.attack_hook;
	if (system_hooks.check_skill_hook != NULL)
		delete system_hooks.check_skill_hook ;
	if (system_hooks.combat_advancement_hook != NULL)
		delete system_hooks.combat_advancement_hook;
	if (system_hooks.get_book_page_hook != NULL)
		delete system_hooks.get_book_page_hook;
	if (system_hooks.hitmiss_hook != NULL)
		delete system_hooks.hitmiss_hook;
	if (system_hooks.open_spellbook_hook != NULL)
		delete system_hooks.open_spellbook_hook;
	if (system_hooks.parry_advancement_hook != NULL)
		delete system_hooks.parry_advancement_hook;
	if (system_hooks.pushthrough_hook != NULL)
		delete system_hooks.pushthrough_hook;
	if (system_hooks.speechmul_hook != NULL)
		delete system_hooks.speechmul_hook;
	if (system_hooks.on_cast_hook != NULL)
		delete system_hooks.on_cast_hook;
}

ExportScript* FindExportScript( const ScriptDef& sd )
{
	for( unsigned i = 0; i < export_scripts.size(); ++i )
	{
		ExportScript* ps = export_scripts[i];
		if (ps->scriptname() == sd.name())
			return ps;
	}

	ExportScript* ps = new ExportScript(sd);
	if (ps->Initialize())
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
ExportedFunction* FindExportedFunction( ConfigElem& elem, 
										const Package* pkg, 
										const std::string& descriptor,
										unsigned nargs,
										bool complain_if_missing )
{
	// first, split up script and functionname
	string::size_type colon_pos = descriptor.find_last_of( ':' );
	if (colon_pos == string::npos)
		elem.throw_error( "Expected ':' in exported function descriptor (" + descriptor + ")" );

	string scriptname = descriptor.substr( 0, colon_pos );
	string functionname = descriptor.substr( colon_pos+1, string::npos );

	ScriptDef sd( scriptname, pkg, "" );

	ExportScript* export_script = FindExportScript( sd );
	if (!export_script)
	{
		if(complain_if_missing)
			elem.throw_error( "Export Script " + sd.name() + " not found" );
		else
			return NULL;
	}

	unsigned PC;
	if (!export_script->FindExportedFunction( functionname, nargs, PC ))
	{
		if(complain_if_missing)
			elem.throw_error( "Exported Function " + functionname + " not found in " + export_script->scriptname() );
		else
			return NULL;
	}

	return new ExportedFunction( export_script, PC );
}
