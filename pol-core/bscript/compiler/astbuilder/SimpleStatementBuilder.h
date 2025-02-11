#ifndef POLSERVER_SIMPLESTATEMENTBUILDER_H
#define POLSERVER_SIMPLESTATEMENTBUILDER_H

#include "bscript/compiler/astbuilder/ExpressionBuilder.h"

namespace Pol::Bscript::Compiler
{
class ConstDeclaration;
class EnumDeclaration;
class JumpStatement;
class Node;
class ReturnStatement;
class Statement;

class SimpleStatementBuilder : public ExpressionBuilder
{
public:
  SimpleStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_intrusive_debug_marker( antlr4::ParserRuleContext*,
                                   std::vector<std::unique_ptr<Statement>>& );

  void add_var_statements( EscriptGrammar::EscriptParser::VarStatementContext*,
                           const std::string& class_name,
                           std::vector<std::unique_ptr<Statement>>& );


  std::unique_ptr<Expression> binding_initializer(
      EscriptGrammar::EscriptParser::BindingDeclarationInitializerContext* );

  std::unique_ptr<Node> binding_list( EscriptGrammar::EscriptParser::BindingDeclarationContext* );

  std::unique_ptr<JumpStatement> break_statement(
      EscriptGrammar::EscriptParser::BreakStatementContext* );

  std::unique_ptr<ConstDeclaration> const_declaration(
      EscriptGrammar::EscriptParser::ConstStatementContext* );

  std::unique_ptr<JumpStatement> continue_statement(
      EscriptGrammar::EscriptParser::ContinueStatementContext* ctx );

  std::unique_ptr<EnumDeclaration> enum_declaration(
      EscriptGrammar::EscriptParser::EnumStatementContext* );

  std::unique_ptr<Expression> variable_initializer(
      EscriptGrammar::EscriptParser::VariableDeclarationInitializerContext* );

  std::unique_ptr<ReturnStatement> return_statement(
      EscriptGrammar::EscriptParser::ReturnStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STATEMENTBUILDER_H
