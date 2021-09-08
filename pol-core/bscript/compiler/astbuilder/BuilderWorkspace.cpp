#include "BuilderWorkspace.h"

#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
BuilderWorkspace::BuilderWorkspace( CompilerWorkspace& compiler_workspace,
                                    SourceFileCache& em_cache, SourceFileCache& inc_cache,
                                    Profile& profile, Report& report, bool is_diagnostics_mode )
  : compiler_workspace( compiler_workspace ),
    em_cache( em_cache ),
    inc_cache( inc_cache ),
    profile( profile ),
    report( report ),
    function_resolver( report ),
    is_diagnostics_mode ( is_diagnostics_mode )
{
}

BuilderWorkspace::~BuilderWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
