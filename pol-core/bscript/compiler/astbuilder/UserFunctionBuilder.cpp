#include "UserFunctionBuilder.h"

namespace Pol::Bscript::Compiler
{
UserFunctionBuilder::UserFunctionBuilder( const SourceFileIdentifier& source_file_identifier,
                                          BuilderWorkspace& workspace )
  : CompoundStatementBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
