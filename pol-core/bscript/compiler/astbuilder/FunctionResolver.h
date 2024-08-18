#ifndef POLSERVER_FUNCTIONRESOLVER_H
#define POLSERVER_FUNCTIONRESOLVER_H

#include <map>
#include <string>
#include <vector>

#include <EscriptGrammar/EscriptParser.h>

#include "bscript/compiler/astbuilder/AvailableParseTree.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ScopableName.h"
#include "bscript/compiler/model/ScopeName.h"
#include "clib/maputil.h"

namespace Pol::Bscript::Compiler
{
struct AvailableParseTree;
class ClassDeclaration;
class ClassLink;
class Function;
class FunctionLink;
class ModuleFunctionDeclaration;
class Report;
class UserFunction;
class SuperFunction;

class FunctionResolver
{
public:
  explicit FunctionResolver( Report& );
  ~FunctionResolver();

  // Force reference to a function
  void force_reference( const ScopableName& name, const SourceLocation& );

  // Force reference to a class
  void force_reference( const ScopeName& name, const SourceLocation& );

  void register_available_generated_function( const SourceLocation&, const ScopableName& name,
                                              Node* context );

  void register_available_user_function( const SourceLocation&,
                                         EscriptGrammar::EscriptParser::FunctionDeclarationContext*,
                                         bool force_reference );

  void register_available_scoped_function(
      const SourceLocation&, const ScopeName& scope,
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* );

  void register_available_class_declaration(
      const SourceLocation& class_loc, const ScopeName& class_name,
      EscriptGrammar::EscriptParser::ClassDeclarationContext* class_ctx,
      Node* top_level_statements_child_node );

  void register_function_link( const ScopableName& name,
                               std::shared_ptr<FunctionLink> function_link );
  void register_class_link( const ScopeName& name, std::shared_ptr<ClassLink> class_link );
  std::string register_function_expression(
      const SourceLocation&, EscriptGrammar::EscriptParser::FunctionExpressionContext* );
  void register_module_function( ModuleFunctionDeclaration* );
  void register_user_function( const std::string& scope, UserFunction* );
  void register_class_declaration( ClassDeclaration* );

  bool resolve( std::vector<AvailableParseTree>& user_functions_to_build );

  static std::string function_expression_name( const SourceLocation& );

private:
  void register_available_function_parse_tree( const SourceLocation&, const ScopableName& name,
                                               const AvailableParseTree& apt );
  void register_available_user_function_parse_tree( const SourceLocation&,
                                                    antlr4::ParserRuleContext*,
                                                    const ScopableName& name,
                                                    bool force_reference );
  void register_available_class_decl_parse_tree( const SourceLocation&, antlr4::ParserRuleContext*,
                                                 const ScopeName& scope,
                                                 Node* top_level_statements_child_node );

  Report& report;

  using FunctionMap = std::map<ScopableName, Function*>;
  using ClassDeclarationMap = std::map<ScopeName, ClassDeclaration*>;

  using FunctionReferenceMap = std::map<ScopableName, std::vector<std::shared_ptr<FunctionLink>>>;
  using ClassReferenceMap = std::map<ScopeName, std::vector<std::shared_ptr<ClassLink>>>;

  using AvailableParseTreeMap = std::map<std::string, AvailableParseTree, Clib::ci_cmp_pred>;

  AvailableParseTreeMap available_user_function_parse_trees;
  AvailableParseTreeMap available_class_decl_parse_trees;
  FunctionMap resolved_functions;
  ClassDeclarationMap resolved_classes;
  FunctionReferenceMap unresolved_function_links;
  ClassReferenceMap unresolved_classes;

  // Returns Function if the key's `{call_scope, name}` exists in
  // `resolved_functions_by_name`, otherwise nullptr.
  Function* check_existing( const ScopableName& key, bool requires_constructor ) const;
  ClassDeclaration* check_existing( const ScopeName& key ) const;

  // Checks if `{call_scope, name}` exists in either
  // `available_user_function_parse_trees` as a function or a class (that would
  // eventually provide the function) inside `available_class_decl_parse_trees`,
  // using `calling_scope` for context.
  bool build_if_available( std::vector<AvailableParseTree>& to_build_ast,
                           const std::string& calling_scope, const ScopableName& call );
  bool build_if_available( std::vector<AvailableParseTree>& to_build_ast, const ScopeName& call );

  // Given a scoped name, looks for an existing function in `resolved_functions`. If found, links
  // the function and returns `true`; otherwise, returns `false`.
  bool resolve_if_existing( const ScopableName&, std::shared_ptr<FunctionLink>& );
  bool resolve_if_existing( const ScopeName&, std::shared_ptr<ClassLink>& );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONRESOLVER_H
