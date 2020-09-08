#ifndef POLSERVER_COMPOUNDSTATEMENTBUILDER_H
#define POLSERVER_COMPOUNDSTATEMENTBUILDER_H

#include "compiler/astbuilder/SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class BasicForLoop;
class Block;
class Statement;
class CaseStatement;
class ConstDeclaration;
class CstyleForLoop;
class DoWhileLoop;
class ForeachLoop;
class RepeatUntilLoop;
class ReturnStatement;
class WhileLoop;

class CompoundStatementBuilder : public SimpleStatementBuilder
{
public:
  CompoundStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_statements( EscriptGrammar::EscriptParser::StatementContext*,
                       std::vector<std::unique_ptr<Statement>>& );

  std::unique_ptr<BasicForLoop> basic_for_loop(
      EscriptGrammar::EscriptParser::BasicForStatementContext*, std::string label );

  std::unique_ptr<Block> block( EscriptGrammar::EscriptParser::BlockContext* );

  std::vector<std::unique_ptr<Statement>> block_statements(
      EscriptGrammar::EscriptParser::BlockContext* );

  std::unique_ptr<CaseStatement> case_statement(
      EscriptGrammar::EscriptParser::CaseStatementContext* );

  std::unique_ptr<CstyleForLoop> cstyle_for_loop(
      EscriptGrammar::EscriptParser::CstyleForStatementContext*, std::string label );

  std::unique_ptr<DoWhileLoop> do_while_loop( EscriptGrammar::EscriptParser::DoStatementContext* );

  std::unique_ptr<Statement> for_loop( EscriptGrammar::EscriptParser::ForGroupContext*,
                                       std::string label );

  std::unique_ptr<Expression> foreach_iterable_expression(
      EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* );

  std::unique_ptr<ForeachLoop> foreach_loop(
      EscriptGrammar::EscriptParser::ForeachStatementContext* );

  std::unique_ptr<Statement> if_statement( EscriptGrammar::EscriptParser::IfStatementContext* );

  std::unique_ptr<RepeatUntilLoop> repeat_until_loop(
      EscriptGrammar::EscriptParser::RepeatStatementContext* );

  std::unique_ptr<WhileLoop> while_loop( EscriptGrammar::EscriptParser::WhileStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPOUNDSTATEMENTBUILDER_H
