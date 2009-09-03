/*
History
=======


Notes
=======

*/

#ifndef SCRSTORE_H
#define SCRSTORE_H

#include <map>
#include <string>

#include "bscript/eprog.h"
#include "clib/refptr.h"
#include "clib/maputil.h"

typedef map< string, ref_ptr<EScriptProgram>, ci_cmp_pred > ScriptStorage;

extern ScriptStorage scrstore;

class ScriptDef;

ref_ptr<EScriptProgram> find_script( const string& name, 
                                     bool complain_if_not_found = true,
                                     bool cache_script = true );

// find_script2: assumes all directory info is there, along with ".ecl"
ref_ptr<EScriptProgram> find_script2( const ScriptDef& script, 
                                      bool complain_if_not_found = true,
                                      bool cache_script = true );


int unload_script( const string& name );
int unload_all_scripts(); // returns # of scripts unloaded
void log_all_script_cycle_counts( bool clear_counters );
void clear_script_profile_counters();

bool script_loaded( ScriptDef& sd );

#endif // SCRSTORE_H
