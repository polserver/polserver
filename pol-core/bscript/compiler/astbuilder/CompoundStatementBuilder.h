#ifndef POLSERVER_COMPOUNDSTATEMENTBUILDER_H
#define POLSERVER_COMPOUNDSTATEMENTBUILDER_H

#include "compiler/astbuilder/SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{

class CompoundStatementBuilder : public SimpleStatementBuilder
{
public:
  CompoundStatementBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  void add_statements( EscriptGrammar::EscriptParser::StatementContext*,
                       std::vector<std::unique_ptr<Statement>>& );

  std::vector<std::unique_ptr<Statement>> block_statements(
      EscriptGrammar::EscriptParser::BlockContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPOUNDSTATEMENTBUILDER_H
