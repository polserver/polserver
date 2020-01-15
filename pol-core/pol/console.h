
#ifndef POL_CONSOLE_H
#define POL_CONSOLE_H

#include "../clib/weakptr.h"
#include <memory>
#include <string>

namespace Pol
{
namespace Clib
{
class ConfigElem;
class KeyboardHook;
}  // namespace Clib
namespace Core
{
class UOExecutor;

class ConsoleReader
{
public:
  ConsoleReader( weak_ptr<Core::UOExecutor> uoexec, bool echo );

#ifdef _WIN32
  void read( std::unique_ptr<ConsoleReader>& holder );
#else
  void read( Clib::KeyboardHook* kb, std::unique_ptr<ConsoleReader>& holder );
#endif

private:
  void revive( const std::string& line, std::unique_ptr<ConsoleReader>& holder );
  weak_ptr<Core::UOExecutor> _uoexec;
  bool _echo;
};

class ConsoleCommand
{
public:
  ConsoleCommand( Clib::ConfigElem& elem, const std::string& cmd );
  size_t estimateSize() const;
  std::string showchar() const;

  char ch;
  std::string script;
  std::string description;

  static ConsoleCommand* find_console_command( char ch );
  static void load_console_commands();
  static void exec_console_cmd( char ch );
#ifdef _WIN32
  static void check_console_commands();
#else
  static void check_console_commands( Clib::KeyboardHook* kb );
#endif

  static bool console_locked;
  static char unlock_char;
};
}  // namespace Core
}  // namespace Pol

#endif
