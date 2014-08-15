/*
History
=======

Notes
=======

*/

#ifndef FMODULE_H
#define FMODULE_H

#include <string>
#include <vector>
#include "options.h"
#include "../clib/maputil.h"
#include "../clib/boostutils.h"

namespace Pol {
  namespace Bscript {
	class ExecutorModule;
	class UserFunction;

	class ModuleFunction
	{
	public:
      boost_utils::function_name_flystring name;
	  unsigned nargs;
	  int funcidx; // according to the executor, what's its function index 

	  // compiler only:
	  UserFunction* uf; // compiler only
	  bool used;          // compiler only

	  ModuleFunction( const char* fname, int nargs, UserFunction* uf );
	  ~ModuleFunction();
	};

	class FunctionalityModule
	{
	public:
	  bool have_indexes;
	  vector<ModuleFunction*> functions;

	  // compiler only:
	  typedef map<std::string, ModuleFunction*, Clib::ci_cmp_pred> FunctionsByName;
	  FunctionsByName functionsByName;
	  vector<ModuleFunction*> used_functions;
	  vector<UserFunction*> owned_userfuncs;

      boost_utils::function_name_flystring modulename;

	  bool isFunc( const char *funcName, ModuleFunction **pmf, int *funcidx );
	  explicit FunctionalityModule( const char* modname );
	  ~FunctionalityModule();

	  void addFunction( const char *funcname, int nparams, UserFunction* uf = NULL );
	  void fillFunctionsByName();

	private:
	  FunctionalityModule( const FunctionalityModule& );
	};
  }
}
#endif
