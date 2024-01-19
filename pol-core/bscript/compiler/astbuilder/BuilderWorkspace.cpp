#include "BuilderWorkspace.h"

#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
BuilderWorkspace::BuilderWorkspace( CompilerWorkspace& compiler_workspace,
                                    SourceFileCache& em_cache, SourceFileCache& inc_cache,
                                    bool continue_on_error, Profile& profile, Report& report )
    : compiler_workspace( compiler_workspace ),
      em_cache( em_cache ),
      inc_cache( inc_cache ),
      continue_on_error( continue_on_error ),
      profile( profile ),
      report( report ),
      function_resolver( report )
{
}

BuilderWorkspace::~BuilderWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
