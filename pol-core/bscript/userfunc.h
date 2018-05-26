/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_USERFUNC_H
#define BSCRIPT_USERFUNC_H

#include <set>
#include <string>
#include <vector>

#include "compctx.h"
#include "token.h"

namespace Pol
{
namespace Bscript
{
class UserParam
{
public:
  std::string name;
  int have_default;
  Token dflt_value;
  bool pass_by_reference;
  bool unused;

  UserParam() : name( "" ), have_default( false ), pass_by_reference( false ), unused( false ) {}
};

class UserFunction
{
public:
  UserFunction();
  ~UserFunction();

  std::string name;
  typedef std::vector<UserParam> Parameters;
  Parameters parameters;
  unsigned position;
  typedef std::vector<unsigned> Addresses;
  Addresses forward_callers;

  CompilerContext ctx;
  char* function_body;  // FIXME: this is leaked
  bool exported;
  bool emitted;
  std::string declaration;

public:
  static unsigned int instances();
  static void show_instances();

protected:
  static unsigned int _instances;
  static std::set<UserFunction*> _instancelist;
  void register_instance();
  void unregister_instance();
};
}
}
#endif
