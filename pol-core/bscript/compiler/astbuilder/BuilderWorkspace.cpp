#include "BuilderWorkspace.h"

#include "compiler/file/SourceFile.h"

namespace Pol::Bscript::Compiler
{
BuilderWorkspace::BuilderWorkspace( CompilerWorkspace& compiler_workspace, Profile& profile,
                                    Report& report )
  : compiler_workspace( compiler_workspace ), profile( profile ), report( report )
{
}

BuilderWorkspace::~BuilderWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
