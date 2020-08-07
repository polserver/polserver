#include "ExpressionBuilder.h"

#include "compiler/Report.h"

namespace Pol::Bscript::Compiler
{
ExpressionBuilder::ExpressionBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
  : ValueBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
