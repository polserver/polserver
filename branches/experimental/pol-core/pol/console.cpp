/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _WIN32
#	include <conio.h>
#else
#	include "clib/kbhit.h"
	keyboard kb;
#endif

#include "../bscript/impstr.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"

#include "polcfg.h"
#include "polsem.h"
#include "polsig.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"

class ConsoleCommand
{
public:
    ConsoleCommand( ConfigElem& elem, const std::string& cmd );

    std::string showchar() const;

    char ch;
    string script;
    string description;
};

std::vector< ConsoleCommand > console_commands;
bool console_locked = true;
char unlock_char;

ConsoleCommand::ConsoleCommand( ConfigElem& elem, const std::string& cmd )
{
    ISTRINGSTREAM is( cmd );
    string charspec;
    if (! (is >> charspec >> script ))
    {
        elem.throw_error( "Ill-formed console command desc: " + cmd );
    }
    if (charspec[0] == '^' && charspec.size() == 2)
    {
        ch = static_cast<char>(toupper( charspec[1] )) - 'A' + 1;
    }
    else if (charspec.size() == 1)
    {
        ch = charspec[0];
    }
    else
    {
        elem.throw_error( "Ill-formed console command char: " + charspec );
    }
    getline( is, description );
    while ( description.size() > 0 && isspace( description[0] ))
    {
        description.erase( 0, 1 );
    }

    if (script == "[unlock]" || script == "[lock/unlock]")
        unlock_char = ch;
}

string getcmdstr( char ch )
{ 
    if (iscntrl(ch)) 
    {
        char s[3];
        s[0] = '^';
        s[1] = ch+'A'-1;
        s[2] = '\0';
        return s;
    }
    else
    {
        char s[2];
        s[0] = ch;
        s[1] = '\0';
        return s;
    }
}

ConsoleCommand* find_console_command( char ch )
{
    for( unsigned i = 0; i < console_commands.size(); ++i )
    {
        ConsoleCommand& cmd = console_commands[i];
        if (cmd.ch == ch)
            return &cmd;
    }
    return NULL;
}

void load_console_commands()
{
    if (!FileExists( "config/console.cfg" ))
        return;

    ConfigFile cf( "config/console.cfg", "Commands" );
    ConfigElem elem;

    console_commands.clear();

    while (cf.read( elem ))
    {
        string tmp;
        while (elem.remove_prop( "CMD", &tmp ))
        {
            ConsoleCommand cmd( elem, tmp );
            console_commands.push_back( cmd );
        }
    }
}

void exec_console_cmd( char ch )
{
#ifdef WIN32
    // cope with function keys.
    if (ch == 0 || static_cast<unsigned char>(ch) == 0xE0)
    {
        getch();
        ch = '?';
    }
#endif
    if (ch == '?')
    {
        cout << "Commands: " << endl;
        for( unsigned i = 0; i < console_commands.size(); ++i )
        {
            ConsoleCommand& cmd = console_commands[i];
            string sc = getcmdstr( cmd.ch );
            if (sc.size() == 1) cout << " ";
            cout << " " << sc << ": ";
            cout << cmd.description << endl;
        }
        cout << "  ?: Help (This list)" << endl;
        return;
    }


    ConsoleCommand* cmd = find_console_command( ch );
    if (!cmd)
    {
        cout << "Unknown console command: '" << getcmdstr(ch) << "'" << endl;
        return;
    }
    if (cmd->script == "[lock]")
    {
        console_locked = true;
        cout << "Console is now locked." << endl;
        return;
    }
    if (cmd->script == "[unlock]")
    {
        console_locked = true;
        cout << "Console is now unlocked." << endl;
        return;
    }
    if (cmd->script == "[lock/unlock]")
    {
        console_locked = !console_locked;
        if (console_locked)
            cout << "Console is now locked." << endl;
        else
            cout << "Console is now unlocked." << endl;
        return;
    }
    if (cmd->script == "[threadstatus]")
    {
        report_status_signalled = true;
        return;
    }
    if (cmd->script == "[crash]")
    {
        int* p = (int*) 17;
        *p = 32;
        return;
    }

    if (console_locked)
    {
        cout << "Console is locked.  Press '" << unlock_char << "' to unlock." << endl;
        return;
    }

    string filename = "scripts/console/" + cmd->script;
    try 
    {
        PolLock lck;
        ScriptDef sd;
        sd.quickconfig( filename + ".ecl" );
        ref_ptr<EScriptProgram> prog = find_script2( sd, true, config.cache_interactive_scripts );
        if (prog.get() != NULL)
            start_script( prog, new String( getcmdstr(ch) ) );
    }
    catch( const char *msg )
    {
        cerr << "Command aborted due to: " << msg << endl;
    }
    catch( string& str )
    {
        cerr << "Command aborted due to: " << str << endl;
    }       // egcs has some trouble realizing 'exception' should catch
    catch( runtime_error& re )   // runtime_errors, so...
    {
        cerr << "Command aborted due to: " << re.what() << endl;
    }
    catch( exception& ex )
    {
        cerr << "Command aborted due to: " << ex.what() << endl;
    }
    catch( int xn )
    {
        cerr << "Command aborted due to: " << xn << endl;
    }
    return;
}

void check_console_commands()
{
#ifdef _WIN32
    if (kbhit())
    {
        exec_console_cmd( static_cast<char>(getch()) );
    }
#else
    if (kb.kbhit())
    {
        exec_console_cmd( kb.getch() );
    }
#endif
}

