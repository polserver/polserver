#ifndef POLSERVER_COMPOUNDSTATEMENTBUILDER_H
#define POLSERVER_COMPOUNDSTATEMENTBUILDER_H

#include "compiler/astbuilder/SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class Block;
class Statement;

class CompoundStatementBuilder : public SimpleStatementBuilder
{
public:
  CompoundStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_statements( EscriptGrammar::EscriptParser::StatementContext*,
                       std::vector<std::unique_ptr<Statement>>& );

  std::unique_ptr<Block> block( EscriptGrammar::EscriptParser::BlockContext* );

  std::vector<std::unique_ptr<Statement>> block_statements(
      EscriptGrammar::EscriptParser::BlockContext* );

  std::unique_ptr<Statement> if_statement( EscriptGrammar::EscriptParser::IfStatementContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPOUNDSTATEMENTBUILDER_H
