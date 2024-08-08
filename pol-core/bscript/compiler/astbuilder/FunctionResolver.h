#ifndef POLSERVER_FUNCTIONRESOLVER_H
#define POLSERVER_FUNCTIONRESOLVER_H

#include <map>
#include <string>
#include <vector>

#include <EscriptGrammar/EscriptParser.h>

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

  const Function* find( const std::string& scoped_name );

  void force_reference( const std::string& name, const SourceLocation& );

  void register_available_user_function(
      const SourceLocation&, EscriptGrammar::EscriptParser::FunctionDeclarationContext* );
  void register_available_class_decl( const SourceLocation&,
                                      EscriptGrammar::EscriptParser::ClassDeclarationContext* );
  void register_function_link( const std::string& name,
                               std::shared_ptr<FunctionLink> function_link );
  std::string register_function_expression(
      const SourceLocation&, EscriptGrammar::EscriptParser::FunctionExpressionContext* );
  void register_module_function( ModuleFunctionDeclaration* );
  void register_user_function( UserFunction* );
  void register_class_declaration( ClassDeclaration* );

  bool resolve( std::vector<AvailableParseTree>& user_functions_to_build );

  static std::string function_expression_name( const SourceLocation& );

private:
  void register_available_user_function_parse_tree( const SourceLocation&,
                                                    antlr4::ParserRuleContext*,
                                                    antlr4::tree::TerminalNode* identifier,
                                                    antlr4::tree::TerminalNode* exported );
  void register_available_class_decl_parse_tree( const SourceLocation&, antlr4::ParserRuleContext*,
                                                 antlr4::tree::TerminalNode* identifier );

  Report& report;

  using AvailableParseTreeMap = std::map<std::string, AvailableParseTree, Clib::ci_cmp_pred>;

  using FunctionMap = std::map<std::string, Function*, Clib::ci_cmp_pred>;
  using ClassDeclarationMap = std::map<std::string, ClassDeclaration*, Clib::ci_cmp_pred>;

  using FunctionReferenceMap =
      std::map<std::string, std::vector<std::shared_ptr<FunctionLink>>, Clib::ci_cmp_pred>;

  AvailableParseTreeMap available_parse_trees;
  FunctionMap resolved_functions_by_name;
  // TODO use this
  ClassDeclarationMap resolved_classes_by_name;
  FunctionReferenceMap unresolved_function_links_by_name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONRESOLVER_H
