/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_CONFIG_H
#define BSCRIPT_CONFIG_H
namespace Pol {
  namespace Bscript {
	struct EScriptConfig
	{
	  unsigned int max_call_depth;
	};

	extern EScriptConfig escript_config;
  }
}
#endif
