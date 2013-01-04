/*
History
=======
2009/10/22 Turley:    added OuchHook
2009/11/26 Turley:    Syshook CanDie(mobile)


Notes
=======

*/

#ifndef SYSHOOK_H
#define SYSHOOK_H

class BObjectImp;
class ConfigElem;
class ExportScript;
class Package;

#include "scrdef.h"
#include "uoexec.h"
class ExportedFunction
{
public:
	ExportedFunction( ExportScript*, unsigned PC );
	~ExportedFunction();
	const std::string& scriptname() const;

	bool call( BObjectImp* p0 ); // throw()
	bool call( BObjectImp* p0, BObjectImp* p1 ); // throw()
	bool call( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 ); // throw()
	bool call( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2, BObjectImp* p3 ); // throw()

	std::string call_string( BObjectImp* p0, BObjectImp* p1 ); // throw()
	std::string call_string( BObjectImp* p0, BObjectImp* p1, BObjectImp* p2 ); // throw()

	int call_long( BObjectImp* p0 ); // throw()
	int call_long( BObjectImp* p0, BObjectImp* p1 ); // throw()
	BObject call_object(BObjectImp* p0, BObjectImp* p1);
	BObject call_object(BObjectImp* p0, BObjectImp* p1, BObjectImp* p2);

private:
	ExportScript* export_script;
	unsigned PC;
};

class SystemHooks
{
public:
	SystemHooks();
	//void clear();

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
extern SystemHooks system_hooks;

ExportedFunction* FindExportedFunction( ConfigElem& elem, 
										const Package* pkg, 
										const std::string& descriptor,
										unsigned nargs,
										bool complain_if_missing = true);

#endif
