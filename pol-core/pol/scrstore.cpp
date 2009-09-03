/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include "bscript/eprog.h"

#include "clib/cistring.h"
#include "clib/logfile.h"
#include "clib/refptr.h"
#include "clib/strutil.h"

#include "polcfg.h"
#include "profile.h"
#include "scrdef.h"

#include "scrstore.h"

bool script_loaded( ScriptDef& sd )
{
    ScriptStorage::iterator itr = scrstore.find( sd.name().c_str() );
    return (itr != scrstore.end());
}

ScriptStorage scrstore;

ref_ptr<EScriptProgram> find_script( const std::string& name, 
                                     bool complain_if_not_found,
                                     bool cache_script)
{
    ScriptStorage::iterator itr = scrstore.find( name.c_str() );
    if (itr != scrstore.end())
    {
        if (cache_script)
        {
            return (*itr).second;
        }
        else
        {
            ref_ptr<EScriptProgram> res( (*itr).second );
            scrstore.erase( itr );
            return res;
        }
    }

    ref_ptr<EScriptProgram> program( new EScriptProgram );
    string pathname = "scripts/";
    pathname += name.c_str();
    if (name.find(".ecl") == string::npos)
        pathname += ".ecl";
    
    if (program->read( pathname.c_str() ) != 0)
    {
        if (complain_if_not_found)
        {
            cerr << "Unable to read script '" << pathname.c_str() << "'" << endl;
            Log( "Unable to read script '%s'\n", pathname.c_str() );
        }
        return ref_ptr<EScriptProgram>(0);
    }

    if (cache_script)
    {
        string tmpname = name;
        mklower( tmpname );
        scrstore.insert( ScriptStorage::value_type( tmpname.c_str(), program ) );
    }

    return program;
}

// NOTE,we assume this has directory info (including scripts/ or pkg/xx)
//      as well as ".ecl" on the end.
ref_ptr<EScriptProgram> find_script2( const ScriptDef& script, 
                                     bool complain_if_not_found,
                                     bool cache_script)
{
    ScriptStorage::iterator itr = scrstore.find( script.c_str() );
    if (itr != scrstore.end())
        return (*itr).second;

    ref_ptr<EScriptProgram> program( new EScriptProgram );
    program->pkg = script.pkg();
    
    if (program->read( script.c_str() ) != 0)
    {
        if (complain_if_not_found)
        {
            cerr << "Unable to read script '" << script.name() << "'" << endl;
            Log( "Unable to read script '%s'\n", script.c_str() );
        }
        return ref_ptr<EScriptProgram>(0);
    }

    if (cache_script)
    {
        string tmpname = script.name();
        mklower( tmpname );
        scrstore.insert( ScriptStorage::value_type( tmpname.c_str(), program ) );
    }

    return program;
}

void preload_test_scripts();
void preload_test_scripts(const std::string&);
int unload_script( const string& name_in )
{
    int n = 0;
    ScriptStorage::iterator itr = scrstore.begin();
    while (itr != scrstore.end())
    {
        ScriptStorage::iterator cur = itr;
        ++itr;
        
        const string& nm = (*cur).first;
		const char* nm_cstr = nm.c_str();
        if (strstr( nm_cstr, name_in.c_str() ))
        {
            cout << "Unloading " << nm_cstr << endl;
            scrstore.erase( cur );
            ++n;

			//dave added 1/30/3, FIXME: if in the future we have to add any other scripts to 
			//auto-reload, put the names in a data structure :P
			if( strstr(nm_cstr,"unequiptest.ecl") )
			{
				preload_test_scripts(string("unequiptest.ecl"));
				continue;
			}
			
			if( strstr(nm_cstr,"equiptest.ecl") )
			{
				preload_test_scripts(string("equiptest.ecl"));
				continue;
			}
        }
    }
    return n;
}

int unload_all_scripts()
{
    int n = scrstore.size();
    scrstore.clear();
	preload_test_scripts(); //dave added 1/30/3, no other time do we reload unequiptest and equiptest
    return n;
}

void log_all_script_cycle_counts( bool clear_counters )
{
    ScriptStorage::iterator itr = scrstore.begin(), end=scrstore.end();
    u64 total_instr = 0;
    for(; itr != end; ++itr )
    {
        EScriptProgram* eprog = ((*itr).second).get();
        total_instr += eprog->instr_cycles;
    }

    itr = scrstore.begin();
    end=scrstore.end();
    if (config.multithread)
    {
        Log( "Scheduler passes: %ld\n", GET_PROFILEVAR(scheduler_passes) );
        Log( "Script passes:    %"OUT64"d\n", script_passes );
    }
    else
    {
        Log( "Total gameloop iterations: %ld\n", rotations );
    }

    Log( "%-38s %12s %6s %12s %6s\n", "Script", "cycles", "invoc", "cyc/invoc", "%" );
    for(; itr != end; ++itr )
    {
        EScriptProgram* eprog = ((*itr).second).get();
        Log( "%-38s %12"OUT64"d %6ld %12"OUT64"d %.2f\n", 
               eprog->name.c_str(), 
               eprog->instr_cycles,
               eprog->invocations,
               eprog->instr_cycles / 
                    (eprog->invocations?eprog->invocations:1),
               static_cast<double>(eprog->instr_cycles) / total_instr * 100.0 );
        if (clear_counters)
        {
            eprog->instr_cycles = 0;
            eprog->invocations = eprog->count() - 1; // 1 count is the scrstore's
        }
    }
    if (clear_counters)
        Log( "Profiling counters cleared.\n" );
}


void clear_script_profile_counters()
{
    ScriptStorage::iterator itr = scrstore.begin(), end=scrstore.end();

    for(; itr != end; ++itr )
    {
        EScriptProgram* eprog = ((*itr).second).get();
        eprog->instr_cycles = 0;
        eprog->invocations = eprog->count() - 1; // 1 count is the scrstore's
    }

    Log( "Profiling counters cleared.\n" );
}

