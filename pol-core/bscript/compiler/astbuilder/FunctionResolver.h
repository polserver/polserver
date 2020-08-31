#ifndef POLSERVER_FUNCTIONRESOLVER_H
#define POLSERVER_FUNCTIONRESOLVER_H

#include <map>
#include <string>
#include <vector>

#include <EscriptGrammar/EscriptParser.h>

#include "clib/maputil.h"
#include "compiler/file/SourceLocation.h"

namespace Pol::Bscript::Compiler
{
struct AvailableUserFunction;
class Function;
class FunctionLink;
class ModuleFunctionDeclaration;
class Report;
class UserFunction;

class FunctionResolver
{
public:
  explicit FunctionResolver( Report& );
  ~FunctionResolver();

  const Function* find( const std::string& scoped_name );

  void register_available_user_function(
      const SourceLocation&, EscriptGrammar::EscriptParser::FunctionDeclarationContext* );
  void register_function_link( const std::string& name,
                               std::shared_ptr<FunctionLink> function_link );
  void register_module_function( ModuleFunctionDeclaration* );
  void register_user_function( UserFunction* );

  bool resolve( std::vector<AvailableUserFunction>& user_functions_to_build );

private:

  void register_available_user_function_parse_tree( const SourceLocation&,
                                                    antlr4::ParserRuleContext*,
                                                    antlr4::tree::TerminalNode* identifier,
                                                    antlr4::tree::TerminalNode* exported );

  Report& report;

  using UserFunctionMap = std::map<std::string, AvailableUserFunction, Clib::ci_cmp_pred>;

  using FunctionMap = std::map<std::string, Function*, Clib::ci_cmp_pred>;

  using FunctionReferenceMap =
      std::map<std::string, std::vector<std::shared_ptr<FunctionLink>>, Clib::ci_cmp_pred>;

  UserFunctionMap available_user_function_parse_trees;
  FunctionMap resolved_functions_by_name;
  FunctionReferenceMap unresolved_function_links_by_name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONRESOLVER_H
