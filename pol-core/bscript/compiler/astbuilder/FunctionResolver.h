#ifndef POLSERVER_FUNCTIONRESOLVER_H
#define POLSERVER_FUNCTIONRESOLVER_H

#include <map>
#include <string>
#include <vector>

#include "../EscriptGrammar/EscriptParser.h"
#include "../clib/maputil.h"

#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
class Function;
class FunctionLink;
class ModuleFunctionDeclaration;
class Report;
class UserFunction;

class FunctionResolver
{
public:
  explicit FunctionResolver( Report& );

  const Function* find( const std::string& scoped_name );

  void register_function_link( const std::string& name,
                               std::shared_ptr<FunctionLink> function_link );
  void register_module_function( ModuleFunctionDeclaration* );

private:

  Report& report;

  //// what about this? (ci_map in Clib)
  // template<typename T>
  // using ci_map = std::map<std::string, T, Clib::ci_cmp_pred>;
  //
  // using AmbiguousUserFunctionMap =
  //    ci_map<EscriptGrammar::EscriptParser::FunctionDeclarationContext*>;

  using FunctionMap = std::map<std::string, Function*, Clib::ci_cmp_pred>;

  using FunctionReferenceMap =
      std::map<std::string, std::vector<std::shared_ptr<FunctionLink>>, Clib::ci_cmp_pred>;

  FunctionMap resolved_functions_by_name;
  FunctionReferenceMap unresolved_function_links_by_name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONRESOLVER_H
