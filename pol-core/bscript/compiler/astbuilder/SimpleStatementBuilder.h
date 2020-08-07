#ifndef POLSERVER_SIMPLESTATEMENTBUILDER_H
#define POLSERVER_SIMPLESTATEMENTBUILDER_H

#include "ExpressionBuilder.h"

namespace Pol::Bscript::Compiler
{
class Statement;
class BreakStatement;
class ConstDeclaration;
class ContinueStatement;
class EnumDeclaration;
class ReturnStatement;

class SimpleStatementBuilder : public ExpressionBuilder
{
public:
  SimpleStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_intrusive_debug_marker( antlr4::ParserRuleContext*,
                                   std::vector<std::unique_ptr<Statement>>& );

  void add_var_statements( EscriptGrammar::EscriptParser::VarStatementContext*,
                           std::vector<std::unique_ptr<Statement>>& );
  void add_var_statements( EscriptGrammar::EscriptParser::UnambiguousVarStatementContext*,
                           std::vector<std::unique_ptr<Statement>>& );

  std::unique_ptr<BreakStatement> break_statement(
      EscriptGrammar::EscriptParser::BreakStatementContext* );

  std::unique_ptr<ConstDeclaration> const_declaration(
      EscriptGrammar::EscriptParser::ConstStatementContext* );
  std::unique_ptr<ConstDeclaration> const_declaration(
      EscriptGrammar::EscriptParser::UnambiguousConstStatementContext* );

  static std::unique_ptr<Statement> consume_statement_result(
      std::unique_ptr<Statement> statement );

  std::unique_ptr<ContinueStatement> continue_statement(
      EscriptGrammar::EscriptParser::ContinueStatementContext* ctx );

  std::unique_ptr<EnumDeclaration> enum_declaration(
      EscriptGrammar::EscriptParser::EnumStatementContext* );
  std::unique_ptr<EnumDeclaration> enum_declaration(
      EscriptGrammar::EscriptParser::UnambiguousEnumStatementContext* );

  std::unique_ptr<Expression> variable_initializer(
      EscriptGrammar::EscriptParser::VariableDeclarationInitializerContext* );
  std::unique_ptr<Expression> variable_initializer(
      EscriptGrammar::EscriptParser::UnambiguousVariableDeclarationInitializerContext* );

  std::unique_ptr<ReturnStatement> return_statement(
      EscriptGrammar::EscriptParser::ReturnStatementContext* );
  std::unique_ptr<ReturnStatement> return_statement(
      EscriptGrammar::EscriptParser::UnambiguousReturnStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STATEMENTBUILDER_H
