#include "SimpleStatementBuilder.h"

#include "compiler/Report.h"
#include "compiler/astbuilder/BuilderWorkspace.h"

namespace Pol::Bscript::Compiler
{
SimpleStatementBuilder::SimpleStatementBuilder( const SourceFileIdentifier& source_file_identifier,
                                    BuilderWorkspace& workspace )
  : ExpressionBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
