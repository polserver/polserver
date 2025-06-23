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
#include "../clib/fileutil.h"
#include "../clib/kbhit.h"
#include "../clib/logfacility.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../plib/systemstate.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "polsem.h"
#include "polsig.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include <fmt/format.h>

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
    std::string tmp = "Commands: \n";
    for ( unsigned i = 0; i < gamestate.console_commands.size(); ++i )
    {
      ConsoleCommand& cmd = gamestate.console_commands[i];
      std::string sc = getcmdstr( cmd.ch );
      tmp += fmt::format( "{}{}: {}\n", sc.size() == 1 ? "  " : " ", sc, cmd.description );
    }
    tmp += "  ?: Help (This list)";
    INFO_PRINTLN( tmp );
    return;
  }


  ConsoleCommand* cmd = find_console_command( ch );
  if ( !cmd )
  {
    INFO_PRINTLN( "Unknown console command: '{}'", getcmdstr( ch ) );
    return;
  }
  if ( cmd->script == "[lock]" )
  {
    ConsoleCommand::console_locked = true;
    INFO_PRINTLN( "Console is now locked." );
    return;
  }
  if ( cmd->script == "[unlock]" )
  {
    ConsoleCommand::console_locked = true;
    INFO_PRINTLN( "Console is now unlocked." );
    return;
  }
  if ( cmd->script == "[lock/unlock]" )
  {
    ConsoleCommand::console_locked = !ConsoleCommand::console_locked;
    if ( ConsoleCommand::console_locked )
      INFO_PRINTLN( "Console is now locked." );
    else
      INFO_PRINTLN( "Console is now unlocked." );
    return;
  }
  if ( cmd->script == "[threadstatus]" )
  {
    stateManager.polsig.report_status_signalled = true;
    return;
  }
  if ( cmd->script == "[crash]" )
  {
    std::abort();
    return;
  }

  if ( ConsoleCommand::console_locked )
  {
    INFO_PRINTLN( "Console is locked.  Press '{}' to unlock.", ConsoleCommand::unlock_char );
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
    ERROR_PRINTLN( "Command aborted due to: {}", msg );
  }
  catch ( std::string& str )
  {
    ERROR_PRINTLN( "Command aborted due to: {}", str );
  }                                 // egcs has some trouble realizing 'exception' should catch
  catch ( std::runtime_error& re )  // runtime_errors, so...
  {
    ERROR_PRINTLN( "Command aborted due to: {}", re.what() );
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN( "Command aborted due to: {}", ex.what() );
  }
  catch ( int xn )
  {
    ERROR_PRINTLN( "Command aborted due to: {}", xn );
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
