#include "ProgramBuilder.h"

namespace Pol::Bscript::Compiler
{
ProgramBuilder::ProgramBuilder( const SourceFileIdentifier& source_file_identifier,
                                BuilderWorkspace& workspace )
  : CompoundStatementBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
