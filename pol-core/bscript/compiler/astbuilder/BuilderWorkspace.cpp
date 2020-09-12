#include "BuilderWorkspace.h"

#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileCache.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/astbuilder/AvailableUserFunction.h"
#include "compiler/file/SourceFileIdentifier.h"

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
    function_resolver( report )
{
}

BuilderWorkspace::~BuilderWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
