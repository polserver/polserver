#include "ModuleDeclarationBuilder.h"

namespace Pol::Bscript::Compiler
{
ModuleDeclarationBuilder::ModuleDeclarationBuilder(
    const SourceFileIdentifier& source_file_identifier, BuilderWorkspace& workspace )
  : SimpleStatementBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
