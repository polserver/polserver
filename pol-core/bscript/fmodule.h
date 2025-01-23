/** @file
 *
 * @par History
 */


#ifndef FMODULE_H
#define FMODULE_H

#include <map>
#include <stddef.h>
#include <string>
#include <vector>

#include "../clib/boostutils.h"
#include "../clib/maputil.h"

namespace Pol
{
namespace Bscript
{
class UserFunction;

class ModuleFunction
{
public:
  boost_utils::function_name_flystring name;
  unsigned nargs;
  int funcidx;  // according to the executor, what's its function index

  ModuleFunction( const char* fname, int nargs );
  ~ModuleFunction() = default;
};

class FunctionalityModule
{
public:
  bool have_indexes;
  std::vector<ModuleFunction*> functions;


  boost_utils::function_name_flystring modulename;

  explicit FunctionalityModule( const char* modname );
  FunctionalityModule( const FunctionalityModule& ) = delete;
  ~FunctionalityModule();

  void addFunction( const char* funcname, int nparams );
};
}  // namespace Bscript
}  // namespace Pol
#endif
