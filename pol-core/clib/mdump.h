/*
History
=======

Notes
=======

*/

#ifndef __MDUMP_H
#define __MDUMP_H

namespace Pol {
  namespace Clib {
	class MiniDumper
	{
	public:
	  static void Initialize();
	  static void SetMiniDumpType( const std::string& dumptype );
	};
  }
}
#endif
