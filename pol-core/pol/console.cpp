/** @file
 *
 * @par History
 */


#include "console.h"

#include <ctype.h>
#include <stddef.h>

#include "../bscript/eprog.h"
#include "../bscript/impstr.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/kbhit.h"
#include "../clib/logfacility.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../clib/weakptr.h"
#include "../plib/systemstate.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "polsem.h"
#include "polsig.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "uoexec.h"
#include <format/format.h>
#include <iostream>
#include <memory>

#ifdef _WIN32
#include <conio.h>
#endif

#include <exception>
#include <stdexcept>
#include <string>

namespace Pol
{
namespace Core
{
bool ConsoleCommand::console_locked = true;
char ConsoleCommand::unlock_char;

ConsoleReader::ConsoleReader( weak_ptr<UOExecutor> uoexec, bool echo )
    : _uoexec( uoexec ), _echo( echo )
{
}

void ConsoleReader::revive( const std::string& line, std::unique_ptr<ConsoleReader>& holder )
{
  if ( _uoexec.exists() )
  {
    PolLock lock;
    Core::UOExecutor* uoexec = _uoexec.get_weakptr();
    uoexec->ValueStack.back().set( new Bscript::BObject( new Bscript::String( line ) ) );
    // Reset the holder first, because reviving the script could call ConsoleInput again,
    // and we use `holder` to know if there is an active script waiting for console input
    holder.reset();
    uoexec->revive();
  }
  else
  {
    DEBUGLOG << "ConsoleInput Script has been destroyed\n";
    holder.reset();
  }
}

#ifdef WIN32
void ConsoleReader::read( std::unique_ptr<ConsoleReader>& holder )
{
  std::string line;
  while ( !Clib::exit_signalled )
  {
    Core::pol_sleep_ms( 100 );
    if ( kbhit() )
    {
      char ch = static_cast<char>( getch() );

      if ( static_cast<unsigned char>( ch ) == 0xE0 )
      {
        getch();
        continue;
      }

      if ( ch == 13 || ch == 10 )
      {
        std::cout << std::endl;
        revive( line, holder );
        break;
      }
      else
      {
        if ( _echo )
          std::cout << ch;
        
        if ( ch == '\b' )
          line.pop_back();
        else
          line += ch;
      }
    }
  }
}
#else
void ConsoleReader::read( Clib::KeyboardHook* kb, std::unique_ptr<ConsoleReader>& holder )
{
  std::string line;
  while ( !Clib::exit_signalled )
  {
    Core::pol_sleep_ms( 100 );
    if ( kb->kbhit() )
    {
      char ch = static_cast<char>( kb->getch() );

      if ( ch == 13 || ch == 10 )
      {
        std::cout << std::endl;
        revive( line, holder );
        break;
      }
      else
      {
        if ( _echo )
          std::cout << ch;
        line += ch;
      }
    }
  }
}

#endif

ConsoleCommand::ConsoleCommand( Clib::ConfigElem& elem, const std::string& cmd )
{
  ISTRINGSTREAM is( cmd );
  std::string charspec;
  if ( !( is >> charspec >> script ) )
  {
    elem.throw_error( "Ill-formed console command desc: " + cmd );
  }
  if ( charspec[0] == '^' && charspec.size() == 2 )
  {
    ch = static_cast<char>( toupper( charspec[1] ) ) - 'A' + 1;
  }
  else if ( charspec.size() == 1 )
  {
    ch = charspec[0];
  }
  else
  {
    elem.throw_error( "Ill-formed console command char: " + charspec );
  }
  getline( is, description );
  while ( description.size() > 0 && isspace( description[0] ) )
  {
    description.erase( 0, 1 );
  }

  if ( script == "[unlock]" || script == "[lock/unlock]" )
    ConsoleCommand::unlock_char = ch;
}

size_t ConsoleCommand::estimateSize() const
{
  return sizeof( char ) + script.capacity() + description.capacity();
}

std::string getcmdstr( char ch )
{
  if ( iscntrl( ch ) )
  {
    char s[3];
    s[0] = '^';
    s[1] = ch + 'A' - 1;
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

ConsoleCommand* ConsoleCommand::find_console_command( char ch )
{
  for ( unsigned i = 0; i < gamestate.console_commands.size(); ++i )
  {
    ConsoleCommand& cmd = gamestate.console_commands[i];
    if ( cmd.ch == ch )
      return &cmd;
  }
  return nullptr;
}

void ConsoleCommand::load_console_commands()
{
  if ( !Clib::FileExists( "config/console.cfg" ) )
    return;

  Clib::ConfigFile cf( "config/console.cfg", "Commands" );
  Clib::ConfigElem elem;

  gamestate.console_commands.clear();

  while ( cf.read( elem ) )
  {
    std::string tmp;
    while ( elem.remove_prop( "CMD", &tmp ) )
    {
      ConsoleCommand cmd( elem, tmp );
      gamestate.console_commands.push_back( cmd );
    }
  }
}

void ConsoleCommand::exec_console_cmd( char ch )
{
#ifdef WIN32
  // cope with function keys.
  if ( ch == 0 || static_cast<unsigned char>( ch ) == 0xE0 )
  {
    getch();
    ch = '?';
  }
#endif
  if ( ch == '?' )
  {
    fmt::Writer tmp;
    tmp << "Commands: \n";
    for ( unsigned i = 0; i < gamestate.console_commands.size(); ++i )
    {
      ConsoleCommand& cmd = gamestate.console_commands[i];
      std::string sc = getcmdstr( cmd.ch );
      if ( sc.size() == 1 )
        tmp << " ";
      tmp << " " << sc << ": ";
      tmp << cmd.description << "\n";
    }
    tmp << "  ?: Help (This list)\n";
    INFO_PRINT << tmp.str();
    return;
  }


  ConsoleCommand* cmd = find_console_command( ch );
  if ( !cmd )
  {
    INFO_PRINT << "Unknown console command: '" << getcmdstr( ch ) << "'\n";
    return;
  }
  if ( cmd->script == "[lock]" )
  {
    ConsoleCommand::console_locked = true;
    INFO_PRINT << "Console is now locked.\n";
    return;
  }
  if ( cmd->script == "[unlock]" )
  {
    ConsoleCommand::console_locked = true;
    INFO_PRINT << "Console is now unlocked.\n";
    return;
  }
  if ( cmd->script == "[lock/unlock]" )
  {
    ConsoleCommand::console_locked = !ConsoleCommand::console_locked;
    if ( ConsoleCommand::console_locked )
      INFO_PRINT << "Console is now locked.\n";
    else
      INFO_PRINT << "Console is now unlocked.\n";
    return;
  }
  if ( cmd->script == "[threadstatus]" )
  {
    stateManager.polsig.report_status_signalled = true;
    return;
  }
  if ( cmd->script == "[crash]" )
  {
    int* p = (int*)17;
    *p = 32;
    return;
  }

  if ( ConsoleCommand::console_locked )
  {
    INFO_PRINT << "Console is locked.  Press '" << ConsoleCommand::unlock_char << "' to unlock.\n";
    return;
  }

  std::string filename = "scripts/console/" + cmd->script;
  try
  {
    PolLock lck;
    ScriptDef sd;
    sd.quickconfig( filename + ".ecl" );
    ref_ptr<Bscript::EScriptProgram> prog =
        find_script2( sd, true, Plib::systemstate.config.cache_interactive_scripts );
    if ( prog.get() != nullptr )
      start_script( prog, new Bscript::String( getcmdstr( ch ) ) );
  }
  catch ( const char* msg )
  {
    ERROR_PRINT << "Command aborted due to: " << msg << "\n";
  }
  catch ( std::string& str )
  {
    ERROR_PRINT << "Command aborted due to: " << str << "\n";
  }                                 // egcs has some trouble realizing 'exception' should catch
  catch ( std::runtime_error& re )  // runtime_errors, so...
  {
    ERROR_PRINT << "Command aborted due to: " << re.what() << "\n";
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << "Command aborted due to: " << ex.what() << "\n";
  }
  catch ( int xn )
  {
    ERROR_PRINT << "Command aborted due to: " << xn << "\n";
  }
  return;
}
#ifdef _WIN32
void ConsoleCommand::check_console_commands()
{
  if ( kbhit() )
  {
    exec_console_cmd( static_cast<char>( getch() ) );
  }
}
#else
void ConsoleCommand::check_console_commands( Clib::KeyboardHook* kb )
{
  if ( kb->kbhit() )
  {
    exec_console_cmd( kb->getch() );
  }
}
#endif
}  // namespace Core
}  // namespace Pol
