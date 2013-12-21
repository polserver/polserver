/*
History
=======


Notes
=======

*/

#include <string>
#include <vector>

#include "../bscript/bobject.h"
namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Package;
  }
  namespace Core {
	
	class CmdLevel
	{
	public:
	  CmdLevel( Clib::ConfigElem& elem, int cmdlevelnum );

	  bool matches( const std::string& name ) const;
	  void add_searchdir( Plib::Package* pkg, const std::string& dir );
	  void add_searchdir_front( Plib::Package* pkg, const std::string& dir );

	  std::string name;
	  unsigned char cmdlevel;

	  struct SearchDir
	  {
		Plib::Package* pkg;
		std::string dir;
	  };

	  typedef std::vector< SearchDir > SearchList;
	  SearchList searchlist;

	  typedef std::vector< std::string > Aliases;
	  Aliases aliases;
	};

	typedef std::vector< CmdLevel > CmdLevels;
	extern CmdLevels cmdlevels2;

	CmdLevel* find_cmdlevel( const char* name );
	CmdLevel* FindCmdLevelByAlias( const std::string& str );

	Bscript::ObjArray* GetCommandsInPackage( Plib::Package* m_pkg, int cmdlvl_num );

  }
}
