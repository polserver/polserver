#ifndef POLSERVER_COMPOUNDSTATEMENTBUILDER_H
#define POLSERVER_COMPOUNDSTATEMENTBUILDER_H

#include "SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class BasicForLoop;
class Block;
class Statement;
class BreakStatement;
class CaseStatement;
class ConstDeclaration;
class ContinueStatement;
class CstyleForLoop;
class DoWhileLoop;
class EnumDeclaration;
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
  void add_statements( EscriptGrammar::EscriptParser::UnambiguousStatementContext*,
                       std::vector<std::unique_ptr<Statement>>& );

  std::unique_ptr<BasicForLoop> basic_for_loop(
      EscriptGrammar::EscriptParser::BasicForStatementContext*, std::string label );
  std::unique_ptr<BasicForLoop> basic_for_loop(
      EscriptGrammar::EscriptParser::UnambiguousBasicForStatementContext*, std::string label );

  std::unique_ptr<Block> block( EscriptGrammar::EscriptParser::BlockContext* );
  std::unique_ptr<Block> block( EscriptGrammar::EscriptParser::UnambiguousBlockContext* );

  std::vector<std::unique_ptr<Statement>> block_statements(
      EscriptGrammar::EscriptParser::BlockContext* );
  std::vector<std::unique_ptr<Statement>> block_statements(
      EscriptGrammar::EscriptParser::UnambiguousBlockContext* );

  std::unique_ptr<CaseStatement> case_statement(
      EscriptGrammar::EscriptParser::CaseStatementContext* );
  std::unique_ptr<CaseStatement> case_statement(
      EscriptGrammar::EscriptParser::UnambiguousCaseStatementContext* );

  std::unique_ptr<CstyleForLoop> cstyle_for_loop(
      EscriptGrammar::EscriptParser::CstyleForStatementContext*, std::string label );
  std::unique_ptr<CstyleForLoop> cstyle_for_loop(
      EscriptGrammar::EscriptParser::UnambiguousCstyleForStatementContext*, std::string label );

  std::unique_ptr<DoWhileLoop> do_while_loop( EscriptGrammar::EscriptParser::DoStatementContext* );
  std::unique_ptr<DoWhileLoop> do_while_loop(
      EscriptGrammar::EscriptParser::UnambiguousDoStatementContext* );

  std::unique_ptr<Statement> for_loop( EscriptGrammar::EscriptParser::ForGroupContext*,
                                       std::string label );
  std::unique_ptr<Statement> for_loop( EscriptGrammar::EscriptParser::UnambiguousForGroupContext*,
                                       std::string label );

  std::unique_ptr<ForeachLoop> foreach_loop(
      EscriptGrammar::EscriptParser::ForeachStatementContext* );
  std::unique_ptr<ForeachLoop> foreach_loop(
      EscriptGrammar::EscriptParser::UnambiguousForeachStatementContext* );

  std::unique_ptr<Statement> if_statement( EscriptGrammar::EscriptParser::IfStatementContext* );
  std::unique_ptr<Statement> if_statement(
      EscriptGrammar::EscriptParser::UnambiguousIfStatementContext* );

  std::unique_ptr<RepeatUntilLoop> repeat_until_loop(
      EscriptGrammar::EscriptParser::RepeatStatementContext* );
  std::unique_ptr<RepeatUntilLoop> repeat_until_loop(
      EscriptGrammar::EscriptParser::UnambiguousRepeatStatementContext* );

  std::unique_ptr<WhileLoop> while_loop( EscriptGrammar::EscriptParser::WhileStatementContext* );
  std::unique_ptr<WhileLoop> while_loop(
      EscriptGrammar::EscriptParser::UnambiguousWhileStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPOUNDSTATEMENTBUILDER_H
