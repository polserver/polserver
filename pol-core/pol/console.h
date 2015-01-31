
#ifndef POL_CONSOLE_H
#define POL_CONSOLE_H

#include <string>

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Core {
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
	  static void check_console_commands();

	  static bool console_locked;
	  static char unlock_char;
	};

  }
}

#endif
