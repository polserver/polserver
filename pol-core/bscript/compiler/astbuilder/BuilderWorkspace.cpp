#include "BuilderWorkspace.h"

#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
BuilderWorkspace::BuilderWorkspace( CompilerWorkspace& compiler_workspace,
                                    SourceFileCache& em_cache, SourceFileCache& inc_cache,
                                    Profile& profile, Report& report )
    : compiler_workspace( compiler_workspace ),
      em_cache( em_cache ),
      inc_cache( inc_cache ),
      profile( profile ),
      report( report ),
      function_resolver( compiler_workspace, report )
{
}

BuilderWorkspace::~BuilderWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
