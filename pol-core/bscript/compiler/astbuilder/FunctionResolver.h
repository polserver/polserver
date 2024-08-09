#ifndef POLSERVER_FUNCTIONRESOLVER_H
#define POLSERVER_FUNCTIONRESOLVER_H

#include <map>
#include <string>
#include <vector>

#include <EscriptGrammar/EscriptParser.h>

#include "bscript/compiler/astbuilder/AvailableParseTree.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "clib/maputil.h"

namespace Pol::Bscript::Compiler
{
struct AvailableParseTree;
class ClassDeclaration;
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

  void force_reference( const std::string& scope, const std::string& name, const SourceLocation& );

  void register_available_user_function( const SourceLocation&,
                                         EscriptGrammar::EscriptParser::FunctionDeclarationContext*,
                                         const std::string& scope );

  void register_available_class_declaration(
      const SourceLocation& class_loc, const std::string& class_name,
      EscriptGrammar::EscriptParser::ClassDeclarationContext* class_ctx );

  void register_function_link( const std::string& call_scope, const std::string& name,
                               std::shared_ptr<FunctionLink> function_link );
  std::string register_function_expression(
      const SourceLocation&, EscriptGrammar::EscriptParser::FunctionExpressionContext* );
  void register_module_function( ModuleFunctionDeclaration* );
  void register_user_function( const std::string& scope, UserFunction* );
  void make_default_constructor( const SourceLocation& class_loc, const std::string& class_name );

  bool resolve( std::vector<AvailableParseTree>& user_functions_to_build );

  static std::string function_expression_name( const SourceLocation& );

private:
  void register_available_user_function_parse_tree( const SourceLocation&,
                                                    antlr4::ParserRuleContext*,
                                                    const std::string& scope,
                                                    antlr4::tree::TerminalNode* identifier,
                                                    antlr4::tree::TerminalNode* exported );
  void register_available_class_decl_parse_tree( const SourceLocation&, antlr4::ParserRuleContext*,
                                                 const std::string& scope );

  Report& report;

  struct FunctionMapKey
  {
    std::string call_scope;
    std::string name;

    bool operator<( const FunctionMapKey& other ) const;
  };

  using FunctionMap = std::map<FunctionMapKey, Function*>;
  using FunctionReferenceMap = std::map<FunctionMapKey, std::vector<std::shared_ptr<FunctionLink>>>;
  using AvailableParseTreeMap = std::map<std::string, AvailableParseTree, Clib::ci_cmp_pred>;

  AvailableParseTreeMap available_user_function_parse_trees;
  AvailableParseTreeMap available_class_decl_parse_trees;
  FunctionMap resolved_functions;
  FunctionReferenceMap unresolved_function_links;

  // Returns Function if the key's `{call_scope, name}` exists in
  // `resolved_functions_by_name`, otherwise nullptr.
  Function* check_existing( const FunctionMapKey& key ) const;

  // Checks if `{call_scope, name})` exists in either
  // `available_user_function_parse_trees` as a function or a class (that would eventually provide
  // the function) inside `available_class_decl_parse_trees`, using `calling_scope` for context.
  bool build_if_available( std::vector<AvailableParseTree>& to_build_ast,
                           const std::string& calling_scope, const FunctionMapKey& call );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONRESOLVER_H
