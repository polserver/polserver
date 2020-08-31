#ifndef POLSERVER_SIMPLESTATEMENTBUILDER_H
#define POLSERVER_SIMPLESTATEMENTBUILDER_H

#include "compiler/astbuilder/ExpressionBuilder.h"

namespace Pol::Bscript::Compiler
{
class Statement;
class ReturnStatement;

class SimpleStatementBuilder : public ExpressionBuilder
{
public:
  SimpleStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_var_statements( EscriptGrammar::EscriptParser::VarStatementContext*,
                           std::vector<std::unique_ptr<Statement>>& );

  static std::unique_ptr<Statement> consume_statement_result(
      std::unique_ptr<Statement> statement );

  std::unique_ptr<Expression> variable_initializer(
      EscriptGrammar::EscriptParser::VariableDeclarationInitializerContext* );

  std::unique_ptr<ReturnStatement> return_statement(
      EscriptGrammar::EscriptParser::ReturnStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STATEMENTBUILDER_H
