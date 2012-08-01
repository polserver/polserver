/*
History
=======

2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/05/16 Shinigami: added Prop Character.Race [RACE_* Constants] to support Elfs
2006/05/27 MuadDib:   added "realm" to base_members and updated loop length for added
                      prop.
2009/01/26 MuadDib:   Added a TON of members. All are up to date as of this date.
2009/08/25 Shinigami: STLport-5.2.1 fix: string array definition and memname definition changed

Notes
=======

*/

#include "../clib/stl_inc.h"


#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"

#include "../clib/esignal.h"
#include "../clib/sckutil.h"
#include "../clib/socketsvc.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/wnsckt.h"

#include "polcfg.h"
#include "poldbg.h"
#include "polsem.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "module/uomod.h"
#include "uoexec.h"
// 14 members
const char* poldbg_base_members[] = {  "x", "y", "z", "name", "objtype", "graphic",
										"serial", "color", "facing", "height", "weight",
										"multi", "realm", "dirty" };
// 27 members
const char* poldbg_itemref_members[] = {"amount", "layer", "container", "usescript",
										 "equipscript", "desc", "movable", "invisible",
										 "decayat", "sellprice", "buyprice", "newbie",
										 "tile_layer", "unequipscript", "item_count",
										 "stackable", "saveonexit", "resist_fire", 
										 "resist_cold", "resist_energy", "resist_poison",
                                         "resist_physical", "resist_fire_mod", "resist_cold_mod",
										 "resist_energy_mod", "resist_poison_mod",
										 "resist_physical_mod"};
// 59 members
const char* poldbg_mobileref_members[] = { "warmode", "gender", "race", "trueobjtype",
											"truecolor", "ar_mod", "hidden", "concealed",
											"frozen", "paralyzed", "poisoned", "stealthsteps",
											"squelched", "dead", "ar", "backpack", "weapon",
											"acctname", "acct", "cmdlevel", "cmdlevelstr",
											"criminal", "ip", "gold", "title_prefix",
											"title_suffix", "title_guild", "title_race",
											"guildid", "guild", "murderer", "attached",
											"reportables", "clientversion", "delay_mod",
											"shield", "uclang", "clientver_detail", "clientinfo",
											"createdat", "opponent", "connected", "trading_with",
											"cursor", "gump", "prompt", "movemode",
											"hitchance_mod", "evasionchance_mod", "resist_fire",
											"resist_cold", "resist_energy", "resist_poison",
											"resist_physical", "resist_fire_mod", "resist_cold_mod",
											"resist_energy_mod", "resist_poison_mod", "resist_physical_mod"};

class DebugContext : public ref_counted
{
public:
    DebugContext();
    ~DebugContext();

    std::string prompt() const;
    typedef std::vector<std::string> Results ;
    bool process( const std::string& cmd, Results& results );
    bool done() const { return _done; }
protected:
    string cmd_attach( unsigned pid );
    string cmd_kill( unsigned pid );
    string cmd_loadsym( unsigned pid );
    string cmd_detach();
    string cmd_quit();
    string cmd_start( const string& rest );
    string cmd_call( const string& rest, Results& results );
    string cmd_state();
	string cmd_pc();
    string cmd_ins( Results& results );
    string cmd_instrace();
    string cmd_stepinto();
    string cmd_stepover();
    string cmd_run();
    string cmd_break();
    string cmd_setbp( const string& rest );
    string cmd_clrbp( const string& rest );
    string cmd_clrallbp();
    string cmd_fileline( const string& rest );
    string cmd_files( Results& results );
    string cmd_stacktrace( Results& results );
    string cmd_filecont( const string& rest, Results& results );
    string cmd_funcprof( const string& rest, Results& results );
    string cmd_localvars( Results& results );
    string cmd_globalvars( Results& results );
    string cmd_localvar( const string& rest );
	string cmd_localvarmembers(  const string& rest, Results& results );
    string cmd_inslist( const string& rest, Results& results );
    string cmd_pidlist( const string& rest, Results& results );
    string cmd_getlocalpacked( const string& rest );
    string cmd_getglobalpacked( const string& rest );
    string cmd_setlocalpacked( const string& rest );
    string cmd_setglobalpacked( const string& rest );
    string cmd_scriptlist( const string& rest, Results& results );
    string cmd_scriptprofile( const string& rest, Results& results );
    string cmd_scriptins( const string& rest, Results& results );
    string cmd_scriptsrc( const string& rest, Results& results );
    string cmd_srcprof( const string& rest, Results& results );
    string cmd_setscript( const string& rest, Results& results );
    string cmd_funclist( const string& rest, Results& results );

private:
    bool _authorized;
    bool _done;
    weak_ptr<UOExecutor> uoexec_wptr;
    ref_ptr<EScriptProgram> _script;

    // not implemented:
    DebugContext( const DebugContext& );
    DebugContext& operator=( const DebugContext& );
};

BApplicObjType debugcontextobjimp_type;
typedef BApplicObj< ref_ptr<DebugContext> > DebugContextObjImpBase;
class DebugContextObjImp : public DebugContextObjImpBase
{
public:
    explicit DebugContextObjImp( ref_ptr<DebugContext> rcdctx );
    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectRef get_member( const char* membername );
};
DebugContextObjImp::DebugContextObjImp( ref_ptr<DebugContext> rcdctx ) :
    DebugContextObjImpBase( &debugcontextobjimp_type, rcdctx )
{
}
const char* DebugContextObjImp::typeOf() const
{
    return "DebugContext";
}
int DebugContextObjImp::typeOfInt() const
{
	return OTDebugContext;
}
BObjectImp* DebugContextObjImp::copy() const
{
    return new DebugContextObjImp( value() );
}
BObjectImp* DebugContextObjImp::call_method( const char* methodname, Executor& ex )
{
    if (stricmp( methodname, "process" ) == 0)
    {
        if (!ex.hasParams(1))
            return new BError( "Not enough parameters" );
        const String* str;
        if (ex.getStringParam( 0, str ))
        {
            vector<string> results;
            value()->process( str->value(), results );
            auto_ptr<ObjArray> arr (new ObjArray);
            for( unsigned i = 0; i < results.size(); ++i )
            {
                arr->addElement( new String( results[i] ) );
            }
			return arr.release();
        }
        else
        {
            return new BError( "Invalid parameter type" );
        }
    }
    return new BError( "undefined" );
}
BObjectRef DebugContextObjImp::get_member( const char* membername )
{
    if (stricmp( membername, "prompt") == 0)
        return BObjectRef(new String( value()->prompt() ));
    else
        return BObjectRef(new BError( "Undefined member" ));
}

BObjectImp* create_debug_context()
{
    DebugContext* dctx = new DebugContext;
    vector<string> tmp;
    dctx->process( "password " + config.debug_password, tmp );
    return new DebugContextObjImp( ref_ptr<DebugContext>(dctx) );
}

DebugContext::DebugContext() :
    _authorized( config.debug_password.empty() ),
    _done(false),
    uoexec_wptr(0)
{
}

DebugContext::~DebugContext()
{
    cmd_detach();
}

string DebugContext::prompt() const
{
    if (!_authorized)
        return "Authorization required.";
    else
        return "Ready.";
}

/// [1] Debugger Commands:
///      password [pwd]   Authorize with password
///      quit             exit debugger
///      start [eclfile]  Start a script
///      call [eclfile] [params:packed array]  call a script
///      attach [pid]     Attach to a running script
///      kill [pid]       Kill a running script
///      detach           Detach from attached script
///      state            State of attached script
///      ins              Show instructions near execution point
///      instrace         Execute next instruction only
///      stepinto         Trace to next statement, into functions if necessary
///      stepover         Trace to next statement, skipping called functions
///      run              Run script full speed (until breakpoint hit if any)
///      break            Break into running script
///      setbp [ins]      Set breakpoint at instruction
///      clrbp [ins]      Clear breakpoint at instruction
///      clrallbp         Clear all breakpoints
///      fileline  {ins}  File # and Line # associated with an instruction
///      files            list the source files used by this script
///      filecont [file] {firstline} {lastline}  get file contents
///      localvars        list local variable names
///      inslist [file] [line]  Return list of instructions assoc with a file/line
///      pidlist [str]    list PIDs and scriptnames matching *str*
///      loadsym [pid]    load symbols for a script
///      scriptlist
///      scriptprofile
///      scriptins [filename]

bool DebugContext::process( const std::string& cmdline, vector<string>& results )
{
    // std::cout << "Debug command is " << cmdline << std::endl;
    try
    {
        results.clear();
        string result;
        
        string cmd, rest;
        splitnamevalue(cmdline,cmd,rest);
        if (!_authorized)
        {
            if (cmd == "password")
            {
                _authorized = (rest == config.debug_password);
                results.push_back( string("Password") + (_authorized?"":" not") + " recognized." );
            }
            return _authorized;
        }

        if (cmd == "attach")        result = cmd_attach( atoi( rest.c_str() ) );
        else if (cmd == "stacktrace") result = cmd_stacktrace( results );
        else if (cmd == "quit")     result = cmd_quit();
        else if (cmd == "detach")   result = cmd_detach();
        else if (cmd == "start")    result = cmd_start( rest );
        else if (cmd == "call")     result = cmd_call( rest, results );
        else if (cmd == "state")    result = cmd_state();
		else if (cmd == "pc")		result = cmd_pc();
        else if (cmd == "ins")      result = cmd_ins( results );
        else if (cmd == "instrace") result = cmd_instrace();
        else if (cmd == "stepinto") result = cmd_stepinto();
        else if (cmd == "stepover") result = cmd_stepover();
        else if (cmd == "run")      result = cmd_run();
        else if (cmd == "break")    result = cmd_break();
        else if (cmd == "setbp")    result = cmd_setbp(rest);
        else if (cmd == "clrbp")    result = cmd_clrbp(rest);
        else if (cmd == "clrallbp") result = cmd_clrallbp();
        else if (cmd == "fileline") result = cmd_fileline(rest);
        else if (cmd == "files")    result = cmd_files(results);
        else if (cmd == "filecont") result = cmd_filecont(rest,results);
        else if (cmd == "localvars")result = cmd_localvars(results);
        else if (cmd == "globalvars") result = cmd_globalvars(results);
        else if (cmd == "localvar") result = cmd_localvar(rest);
		else if (cmd == "localvarmembers") result = cmd_localvarmembers(rest,results);
        else if (cmd == "inslist")  result = cmd_inslist(rest,results);
        else if (cmd == "pidlist")  result = cmd_pidlist(rest,results);
        else if (cmd == "scriptlist") result = cmd_scriptlist(rest,results);
        else if (cmd == "scriptprofile") result = cmd_scriptprofile(rest,results);
        else if (cmd == "scriptins") result = cmd_scriptins( rest, results );
        else if (cmd == "scriptsrc") result = cmd_scriptsrc( rest, results );
        else if (cmd == "srcprof") result = cmd_srcprof( rest, results );
        else if (cmd == "funclist") result = cmd_funclist( rest, results );
        else if (cmd == "setscript") result = cmd_setscript( rest, results );
        else if (cmd == "getlocalpacked")  result = cmd_getlocalpacked(rest);
        else if (cmd == "getglobalpacked") result = cmd_getglobalpacked(rest);
        else if (cmd == "setlocalpacked")  result = cmd_setlocalpacked(rest);
        else if (cmd == "setglobalpacked") result = cmd_setglobalpacked(rest);
        else if (cmd == "kill")     result = cmd_kill( atoi( rest.c_str() ) );
        else if (cmd == "loadsym")  result = cmd_loadsym( atoi(rest.c_str()) );
        else                        result = "Command '" + cmdline + "' not recognized.";

        if (!result.empty())
        {
            results.push_back( result );
        }
        return true;
    }
    catch( exception& ex )
    {
        string text = "Exception thrown while processing command: ";
        text += ex.what();
        results.push_back( text );
        return false;
    }
}

string DebugContext::cmd_stacktrace( Results& results )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";

    UOExecutor* exec = uoexec_wptr.get_weakptr();
    if (!exec->halt())
   	{
    	return "Script must be halted to retreive stack trace.";
   	}
    EScriptProgram* prog = const_cast<EScriptProgram*>(exec->prog());
    prog->read_dbg_file();

	const_cast<EScriptProgram*>(prog)->read_dbg_file();
	if (!prog->debug_loaded)
		return "No debugging information available.";


	stack<BObjectRefVec*> upperLocals2 = exec->upperLocals2;
	stack<ReturnContext> stack = exec->ControlStack;

	unsigned int PC;

	ReturnContext rc;
    rc.PC = exec->PC;
    rc.ValueStackDepth = static_cast<unsigned int>(exec->ValueStack.size());
	stack.push( rc );

	upperLocals2.push(exec->Locals2);

	results.push_back(decint(stack.size()));

	while (!stack.empty()) {
		ReturnContext& rc = stack.top();
		BObjectRefVec* Locals2 = upperLocals2.top();
		PC = rc.PC;
		stack.pop();
		upperLocals2.pop();

		BStruct stackEntry;

		size_t left = Locals2->size();

		results.push_back( decint(PC) );
		results.push_back( decint(prog->dbg_filenum[PC]));
		results.push_back( decint(prog->dbg_linenum[PC]));

		vector<string> results2;
		unsigned block = prog->dbg_ins_blocks[ PC ];
		while (left)
		{
			while (left <= prog->blocks[block].parentvariables)
			{
				block = prog->blocks[block].parentblockidx;
			}
			const EPDbgBlock &progblock = prog->blocks[block];
			size_t varidx = left - 1 - progblock.parentvariables;
			left--;
			BObjectImp* ptr = (*Locals2)[varidx]->impptr();

			results2.push_back(progblock.localvarnames[varidx]+" "+ ptr->pack());
		}
		results.push_back( decint(results2.size() ));

		for ( vector<string>::iterator it=results2.begin() ; it < results2.end(); it++ )
			results.push_back(*it);
	}
	return "";

}
string DebugContext::cmd_attach( unsigned pid )
{
    UOExecutor* uoexec;
    if (find_uoexec( pid, &uoexec ))
    {
        uoexec->attach_debugger();
        uoexec_wptr = uoexec->weakptr;
        EScriptProgram* prog = const_cast<EScriptProgram*>(uoexec->prog());
        prog->read_dbg_file();
        _script.set( prog );
        return "Attached to PID " + tostring(pid) + ".";
    }
    else
    {
        return "PID not found.";
    }
}

string DebugContext::cmd_loadsym( unsigned pid )
{
    UOExecutor* uoexec;
    if (find_uoexec( pid, &uoexec ))
    {
        int res = const_cast<EScriptProgram*>(uoexec->prog())->read_dbg_file();
        if (res)
            return "Failed to load symbols.";
        else
            return "Loaded debug symbols.";
    }
    else
    {
        return "PID not found.";
    }
}

string DebugContext::cmd_kill( unsigned pid )
{
    UOExecutor* uoexec;
    if (find_uoexec( pid, &uoexec ))
    {
        uoexec->seterror( true );
        return "Marked PID " + tostring(pid) + " with an error.";
    }
    else
    {
        return "PID not found.";
    }
}

string DebugContext::cmd_detach()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";

    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    if (uoexec->os_module->in_debugger_holdlist())
        uoexec->os_module->revive_debugged();

    uoexec->detach_debugger();
    uoexec_wptr.clear();
    return "Detached.";
}

string DebugContext::cmd_quit()
{
    _done = true;
    return "Bye.";
}

string DebugContext::cmd_start( const string& rest )
{
    string filename = rest;
    ScriptDef sd;
    if (!sd.config_nodie( filename, NULL, "scripts/" ))
        return "Error in script name.";
    if (!sd.exists())
        return "Script " + sd.name() + " does not exist.";

    UOExecutorModule* new_uoemod = ::start_script( sd, NULL );
    if (new_uoemod == NULL)
    {
        return "Unable to start script";
    }

    UOExecutor* uoexec = static_cast<UOExecutor*>(&new_uoemod->exec);

    return "PID " + tostring( uoexec->os_module->pid() );
}

BObjectImp* run_executor_to_completion( UOExecutor& ex, const ScriptDef& script );

string DebugContext::cmd_call( const string& rest, Results& results )
{
    string filename, parameters_packed;
    splitnamevalue( rest, filename, parameters_packed );

    ScriptDef sd;
    if (!sd.config_nodie( filename, NULL, "scripts/" ))
        return "Error in script name.";
    if (!sd.exists())
        return "Script " + sd.name() + " does not exist.";

    try 
    {
        UOExecutor ex;
        BObject params( new UninitObject );
        if (!parameters_packed.empty())
        {
            params.setimp( BObjectImp::unpack( parameters_packed.c_str() ) );
            if (!params.isa( BObjectImp::OTArray ))
            {
                return "Parameters must be an array.";
            }
            ObjArray* arr = static_cast<ObjArray*>(params.impptr());
            for( int i = static_cast<int>(arr->ref_arr.size()-1); i >= 0; --i )
            {
                ex.pushArg( arr->ref_arr[i]->impptr() );
            }
        }
        BObject ret( run_executor_to_completion( ex, sd ) );
        return "Return value packed: " + ret.impptr()->pack();
    }
    catch( ... )
    {
        return "Exception calling script";
    }
}

string DebugContext::cmd_pidlist( const std::string& rest, Results& results )
{
    string match = strlower( rest );

    for( PidList::const_iterator citr = pidlist.begin(); citr != pidlist.end(); ++citr )
    {
        UOExecutor* uoexec = (*citr).second;
        string name = strlower( uoexec->scriptname() );
        if ( strstr( name.c_str(), match.c_str() ) != NULL)
        {
            results.push_back( decint( (*citr).first ) + " " + uoexec->scriptname() );
        }
    }

    return "";
}

string DebugContext::cmd_scriptlist( const std::string& rest, Results& results )
{
    for( ScriptStorage::const_iterator citr = scrstore.begin(); citr != scrstore.end(); ++citr )
    {
        const char* nm = ((*citr).first).c_str();
        EScriptProgram* eprog = ((*citr).second).get();
        string scriptname = eprog->name;
        results.push_back( nm );
    }
    return "";
}

string DebugContext::cmd_setscript( const string& rest, Results& results )
{
    _script.clear();

//const char* fn = rest.c_str();
    ScriptStorage::iterator itr = scrstore.find( rest );
    if (itr == scrstore.end())
        return "No such script.";
    
    ref_ptr<EScriptProgram> res( (*itr).second );
    EScriptProgram* eprog = res.get();
    if (eprog->read_dbg_file() != 0)
        return "Failed to load symbols.";

    _script = res;
    return "";
}

string DebugContext::cmd_funclist( const string& rest, Results& results )
{
    if (_script.get() == 0)
        return "use setscript first";

    // no parameters.

    for( unsigned i = 0; i < _script->dbg_functions.size(); ++i )
    {
        const EPDbgFunction& func = _script->dbg_functions[i];

        unsigned int cycles = 0;
        for( unsigned pc = func.firstPC; pc <= func.lastPC; ++pc )
        {
            const Instruction& ins = _script->instr[pc];
            cycles += ins.cycles;
        }

        string result = func.name + " " 
                        + decint( func.firstPC) + " " 
                        + decint( func.lastPC ) + " "
                        + decint( cycles );
        results.push_back( result );
    }

    return "";
}

string DebugContext::cmd_srcprof( const string& rest, Results& results )
{
    if (_script.get() == 0)
        return "use setscript first";

    // parameter: file#
    int fileno = atoi( rest.c_str() );

    typedef map< unsigned int, unsigned int > Cycles;
    Cycles cycle_counts; // key is line#, val is cycles

    size_t count = _script->instr.size();

    for( size_t i = 0; i < count; ++i )
    {
        int filenum = _script->dbg_filenum[i];
        if (filenum == fileno)
        {
            int linenum = _script->dbg_linenum[i];
            const Instruction& ins = _script->instr[i];
            cycle_counts[ linenum ] += ins.cycles;
        }
    }

    for( Cycles::iterator itr = cycle_counts.begin(); itr != cycle_counts.end(); ++itr )
    {
        unsigned int linenum = (*itr).first;
        unsigned int cycles = (*itr).second;
        string result = decint( linenum ) + " " + decint( cycles );
        results.push_back( result );
    }

    return "";
}

string DebugContext::cmd_funcprof( const string& rest, Results& results )
{
    if (_script.get() == 0)
        return "use setscript first";

    return "";
}

string DebugContext::cmd_scriptprofile( const string& rest, Results& results )
{
    ScriptStorage::iterator itr = scrstore.find( rest.c_str() );
    if (itr == scrstore.end())
        return "No such script.";
    
    ref_ptr<EScriptProgram> res( (*itr).second );
    EScriptProgram* eprog = res.get();
    size_t count = eprog->instr.size();
    for( size_t i = 0; i < count; ++i )
    {
        const Instruction& ins = eprog->instr[i];
        string result = decint( i ) + " " + decint( ins.cycles );
        results.push_back( result );
    }
    return "";
}

string DebugContext::cmd_scriptins( const string& rest, Results& results )
{
    ScriptStorage::iterator itr = scrstore.find( rest.c_str() );
    if (itr == scrstore.end())
        return "No such script.";
    
    ref_ptr<EScriptProgram> res( (*itr).second );
    EScriptProgram* eprog = res.get();
    if (eprog->read_dbg_file() != 0)
        return "Failed to load symbols.";

    size_t count = eprog->instr.size();
    for( size_t i = 0; i < count; ++i )
    {
        string result = decint(i) + " " + eprog->dbg_get_instruction( i );
        results.push_back( result );
    }
    return "";
}

string get_fileline( EScriptProgram* prog, int filenum, int linenum )
{
    if (filenum == 0 || filenum >= static_cast<int>(prog->dbg_filenames.size()))
        return "";
    ifstream ifs( prog->dbg_filenames[filenum].c_str() );
    string tmp;
    for( int skip = 1; skip < linenum; ++skip )
    {
        if (!getline( ifs, tmp ))
            return "";
    }
    if (getline( ifs, tmp ))
        return tmp;
    else
        return "";
}          

string DebugContext::cmd_scriptsrc( const string& rest, Results& results )
{
    ScriptStorage::iterator itr = scrstore.find( rest.c_str() );
    if (itr == scrstore.end())
        return "No such script.";
    
    ref_ptr<EScriptProgram> res( (*itr).second );
    EScriptProgram* eprog = res.get();
    if (eprog->read_dbg_file() != 0)
        return "Failed to load symbols.";

    int last_filenum = -1;
    int last_linenum = -1;
    size_t count = eprog->instr.size();
    for( size_t ins = 0; ins < count; ++ins )
    {
        int filenum = eprog->dbg_filenum[ins];
        int linenum = eprog->dbg_linenum[ins];
        if (filenum == last_filenum && linenum == last_linenum)
            continue;

        string result = get_fileline( eprog, filenum, linenum );
        if (result != "")
            results.push_back( decint(ins) + " " + result );
        
        last_filenum = filenum;
        last_linenum = linenum;
    }
    return "";
}

string DebugContext::cmd_state()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    return uoexec_wptr.get_weakptr()->state();
}

string DebugContext::cmd_pc()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    return tostring(uoexec_wptr.get_weakptr()->PC);
}

string DebugContext::cmd_ins( vector<string>& results )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    int start = uoexec->PC-5;
    int end = uoexec->PC+5;
    if (start < 0) start = 0;
    if (end >= static_cast<int>(uoexec->nLines)) end = uoexec->nLines-1;
    for( int i = start; i <= end; ++i )
    {
        results.push_back( uoexec->dbg_get_instruction(i) );
    }
    return "";
}

string DebugContext::cmd_instrace()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    if (!uoexec->os_module->in_debugger_holdlist())
        return "Script not ready to trace.";

    uoexec->dbg_ins_trace();
    uoexec->os_module->revive_debugged();
    return "Tracing.";
}

string DebugContext::cmd_stepinto()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    if (!uoexec->os_module->in_debugger_holdlist())
        return "Script not ready to trace.";

    uoexec->dbg_step_into();
    uoexec->os_module->revive_debugged();
    return "Stepping In.";
}

string DebugContext::cmd_stepover()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    if (!uoexec->os_module->in_debugger_holdlist())
        return "Script not ready to trace.";

    uoexec->dbg_step_over();
    uoexec->os_module->revive_debugged();
    return "Stepping Over.";
}

std::string DebugContext::cmd_run()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    if (!uoexec->os_module->in_debugger_holdlist())
        return "Script not ready to trace.";

    uoexec->dbg_run();
    uoexec->os_module->revive_debugged();
    return "Running.";
}
std::string DebugContext::cmd_break()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    //if (!uoexec->os_module->in_debugger_holdlist())
    //    return "Script not ready to trace.";

    uoexec->dbg_break();
    return "Marked for break-into.";
}
std::string DebugContext::cmd_setbp( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    uoexec->dbg_setbp( atoi(rest.c_str()) );
    return "Breakpoint set.";
}
std::string DebugContext::cmd_clrbp( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    uoexec->dbg_clrbp( atoi(rest.c_str()) );
    return "Breakpoint cleared.";
}
std::string DebugContext::cmd_clrallbp()
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    uoexec->dbg_clrallbp();
    return "All breakpoints cleared.";
}
std::string DebugContext::cmd_fileline( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    const EScriptProgram* prog = uoexec->prog();
    if (!prog->debug_loaded)
        return "No debug information available.";

    unsigned ins = uoexec->PC;
    if (!rest.empty())
        ins = atoi( rest.c_str() );
    int filenum, linenum;
    if (prog->dbg_filenum.size() <= ins)
        return "out of range";
    filenum = prog->dbg_filenum[ins];
    if (prog->dbg_linenum.size() <= ins)
        return "out of range";
    linenum = prog->dbg_linenum[ins];
    return decint( filenum ) + " " + decint( linenum );
}

std::string DebugContext::cmd_files( Results& results )
{
    const EScriptProgram* prog = NULL;

    if (_script.get() != 0)
        prog = _script.get();
    else if (uoexec_wptr.exists())
        prog = uoexec_wptr.get_weakptr()->prog();
    else
        return "attach or setscript first";

    if (!prog->debug_loaded)
        return "No debug information available.";
    
    for( unsigned i = 0; i < prog->dbg_filenames.size(); ++i )
    {
        string tmp = decint(i) + " " + prog->dbg_filenames[i];
        results.push_back( tmp );
    }
    return "";
}

std::string DebugContext::cmd_filecont( const std::string& rest, Results& results )
{
    const EScriptProgram* prog = NULL;

    if (_script.get() != 0)
        prog = _script.get();
    else if (uoexec_wptr.exists())
        prog = uoexec_wptr.get_weakptr()->prog();
    else
        return "attach or setscript first";

    if (!prog->debug_loaded)
        return "No debug information available.";

    unsigned filenum, firstline, lastline;

    ISTRINGSTREAM is(rest);
    if (!(is >> filenum))
        return "File # must be specified";
    if (!(is >> firstline))
        firstline = 1; // start at beginning
    if (!(is >> lastline))
        lastline = 0; // all lines

    if (filenum >= prog->dbg_filenames.size())
        return "File # out of range";
    
    ifstream ifs( prog->dbg_filenames[filenum].c_str() );
    string tmp;
    for( unsigned skip = 1; skip < firstline; ++skip )
    {
        if (!getline( ifs, tmp ))
            break;
    }
    while ( (lastline == 0 || firstline++ <= lastline) && getline(ifs,tmp) )
    {
    	tmp.erase(tmp.find_last_not_of("\n\r")+1);
        results.push_back( tmp );
    }
           
    return "";
}
std::string DebugContext::cmd_localvars( Results& results )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    const EScriptProgram* prog = uoexec->prog();
    if (!prog->debug_loaded)
        return "No debug information available.";

    // for now, always asking given the current instruction = PC
    unsigned block = prog->dbg_ins_blocks[ uoexec->PC ];
    size_t left = uoexec->Locals2->size();
    results.resize( left );
    while (left)
    {
        while (left <= prog->blocks[block].parentvariables)
        {
            block = prog->blocks[block].parentblockidx;
        }
        size_t varidx = left - 1 - prog->blocks[block].parentvariables;
        results[left-1] = prog->blocks[block].localvarnames[varidx];
        --left;
    }
    return "";
}
std::string DebugContext::cmd_globalvars( Results& results )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    const EScriptProgram* prog = uoexec->prog();
    if (!prog->debug_loaded)
        return "No debug information available.";

    BObjectRefVec::const_iterator itr = uoexec->Globals2.begin(), end=uoexec->Globals2.end();
    
    for( unsigned idx = 0; itr != end; ++itr,++idx )
    {
        if (prog->globalvarnames.size() > idx)
            results.push_back( prog->globalvarnames[idx].c_str() );
        else
            results.push_back( decint( idx ).c_str() );
    }
    return "";
}
std::string DebugContext::cmd_localvar( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    // const EScriptProgram* prog = uoexec->prog();

    unsigned varidx = atoi( rest.c_str() );
    if (varidx >= uoexec->Locals2->size())
        return "Error: Index out of range"; // vector
    return "Value: " + (*uoexec->Locals2)[varidx]->impref().getStringRep();
}

std::string DebugContext::cmd_localvarmembers( const std::string& rest, Results& results )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();

    unsigned varidx = atoi( rest.c_str() );
    if (varidx >= uoexec->Locals2->size())
        return "Error: Index out of range"; // vector
    BObjectImp& var = (*uoexec->Locals2)[varidx]->impref();

	string strrep = var.getStringRep();
	string varline;
	string str_memname;
	const char* memname;
	int i;
	OSTRINGSTREAM os;
	if( strrep.find("ItemRef") != string::npos )
	{
		for(i=0; i<14; i++) // i = member count for poldbg_base_members
		{
			memname = poldbg_base_members[i];
			os << memname << " " << var.get_member(memname).get()->impptr()->getStringRep();

			results.push_back(OSTRINGSTREAM_STR(os));
			os.str("");
		}
		
		for(i=0; i<27; i++) // i = 27 members
		{
			memname = poldbg_itemref_members[i];
			os << memname << " " << var.get_member(memname).get()->impptr()->getStringRep();

			results.push_back(OSTRINGSTREAM_STR(os));
			os.str("");
		}
	}

	else if( strrep.find("MobileRef") != string::npos )
	{
		for(i=0; i<14; i++) // i = member count for poldbg_base_members
		{
			memname = poldbg_base_members[i];
			os << memname << " " << var.get_member(memname).get()->impptr()->getStringRep();

			results.push_back(OSTRINGSTREAM_STR(os));
			os.str("");
		}
		
		for(i=0; i<59; i++) // i = 59 members
		{
			memname = poldbg_mobileref_members[i];
			os << memname << " " << var.get_member(memname).get()->impptr()->getStringRep();

			results.push_back(OSTRINGSTREAM_STR(os));
			os.str("");
		}
	}
	return "Value: " + strrep;
}

std::string DebugContext::cmd_inslist( const std::string& rest, Results& results )
{
    unsigned filenum, linenum;
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    const EScriptProgram* prog = uoexec->prog();
    if (!prog->debug_loaded)
        return "No debug information available.";

    ISTRINGSTREAM is(rest);
    if (!(is >> filenum))
        return "File # must be specified";
    if (!(is >> linenum))
        return "Line # must be specified";

    if (filenum >= prog->dbg_filenames.size())
        return "File # out of range";
    
    for( unsigned i = 0; i < prog->dbg_filenum.size(); ++i )
    {
        if (prog->dbg_filenum[i] == filenum &&
            prog->dbg_linenum[i] == linenum)
        {
            results.push_back( decint(i) );
        }
        else
        {
            if (!results.empty())
                break;
        }
    }
    return "";
}
string DebugContext::cmd_getlocalpacked( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    // const EScriptProgram* prog = uoexec->prog();

    unsigned varidx = atoi( rest.c_str() );
    if (varidx >= uoexec->Locals2->size())
        return "Error: Index out of range"; 
    return "Value: " + (*uoexec->Locals2)[varidx]->impref().pack();
}
string DebugContext::cmd_getglobalpacked( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    // const EScriptProgram* prog = uoexec->prog();

    unsigned varidx = atoi( rest.c_str() );
    if (varidx >= uoexec->Globals2.size())
        return "Error: Index out of range"; 
    return "Value: " + uoexec->Globals2[varidx]->impref().pack();
}
string DebugContext::cmd_setlocalpacked( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    // const EScriptProgram* prog = uoexec->prog();

    ISTRINGSTREAM is(rest);
    unsigned varidx;
    char space;
    if (! (is >> varidx))
        return "format: setlocalpacked [varidx] [packedvalue]";

    is.unsetf( ios::skipws );
    is >> space;
    is.setf( ios::skipws );

    if (varidx >= uoexec->Locals2->size())
        return "Error: Index out of range"; 
  
    BObjectRef& ref = (*uoexec->Locals2)[varidx];
    BObject& obj = *ref;
    BObjectImp* newimp = BObjectImp::unpack( is );
    if (newimp == NULL)
        return "Error: unable to unpack";

    obj.setimp( newimp );

    return "Value: " + (*uoexec->Locals2)[varidx]->impref().pack();
}
string DebugContext::cmd_setglobalpacked( const std::string& rest )
{
    if (!uoexec_wptr.exists())
        return "No script attached.";
    UOExecutor* uoexec = uoexec_wptr.get_weakptr();
    // const EScriptProgram* prog = uoexec->prog();
    ISTRINGSTREAM is(rest);
    unsigned varidx;
    char space;
    if (! (is >> varidx))
        return "format: setglobalpacked [varidx] [packedvalue]";
    is.unsetf( ios::skipws );
    is >> space;
    is.setf( ios::skipws );

    if (varidx >= uoexec->Globals2.size())
        return "Error: Index out of range"; 
  
    BObjectRef& ref = uoexec->Globals2[varidx];
    BObject& obj = *ref;
    BObjectImp* newimp = BObjectImp::unpack( is );
    if (newimp == NULL)
        return "Error: unable to unpack";

    obj.setimp( newimp );

    return "Value: " + uoexec->Globals2[varidx]->impref().pack();
}



class DebugClientThread : public SocketClientThread
{
public:
    DebugClientThread( SocketListener& SL ) : SocketClientThread(SL) {}
    virtual void run();
};

void DebugClientThread::run()
{
    if (config.debug_local_only)
    {
        if (!_sck.is_local())
        {
            writeline( _sck, "Only accepting connections from localhost." );
            return;
        }
    }
    DebugContext dctx;
    string cmdline;
    vector<string> results;
    while(!dctx.done())
    {
        writeline( _sck, dctx.prompt() );
        if (!readline( _sck, cmdline) )
            break;
        
        bool ret = dctx.process( cmdline, results );
        if (ret)
            writeline( _sck, "Results: " + tostring( results.size() ) );
        else
            writeline( _sck, "Failure: " + tostring( results.size() ) );
        for( unsigned i = 0; i < results.size(); ++i )
        {
            writeline( _sck, results[i] );
        }
    }
}

void debug_listen_thread(void)
{
    if (config.debug_port)
    {
        SocketListener SL( config.debug_port );
        while (!exit_signalled)
        {
            if (SL.GetConnection( 5 ))
            {
                SocketClientThread* p = new DebugClientThread(SL);
                p->start();
            }
        }
    }
}

