#include "CompilerWorkspaceBuilder.h"

#include "BuilderWorkspace.h"
#include "compiler/LegacyFunctionOrder.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
CompilerWorkspaceBuilder::CompilerWorkspaceBuilder( Profile& profile, Report& report )
    : profile( profile ), report( report )
{
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build(
    const std::string& pathname,
    const LegacyFunctionOrder* /*legacy_function_order*/ )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>();
  BuilderWorkspace workspace( *compiler_workspace, profile, report );

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );

  return compiler_workspace;
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
