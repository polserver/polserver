#include "SimpleStatementBuilder.h"

#include "compiler/Report.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/ast/Expression.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/ast/ValueConsumer.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript
{
extern int include_debug;
}
namespace Pol::Bscript::Compiler
{
SimpleStatementBuilder::SimpleStatementBuilder( const SourceFileIdentifier& source_file_identifier,
                                    BuilderWorkspace& workspace )
  : ExpressionBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<Statement> SimpleStatementBuilder::consume_statement_result(
    std::unique_ptr<Statement> statement )
{
  return std::make_unique<ValueConsumer>( statement->source_location, std::move( statement ) );
}

}  // namespace Pol::Bscript::Compiler
