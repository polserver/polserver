#include "CompilerWorkspaceBuilder.h"

#include "compiler/LegacyFunctionOrder.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/astbuilder/SourceFileProcessor.h"
#include "compiler/file/SourceFile.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspaceBuilder::CompilerWorkspaceBuilder( SourceFileCache& em_cache,
                                                    SourceFileCache& inc_cache,
                                                    Profile& profile, Report& report )
    : em_cache( em_cache ), inc_cache( inc_cache ), profile( profile ), report( report )
{
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build(
    const std::string& pathname, const LegacyFunctionOrder* /*legacy_function_order*/ )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>();
  BuilderWorkspace workspace( *compiler_workspace, em_cache, inc_cache, profile, report );

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  {
    report.error( *ident, "Refusing to load '", pathname, "'.\n" );
    return {};
  }

  auto sf = SourceFile::load( *ident, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '", pathname, "'.\n" );
    return {};
  }

  SourceFileProcessor src_processor( *ident, workspace, true );

  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
  workspace.source_files[sf->pathname] = sf;

  compiler_workspace->top_level_statements =
      std::make_unique<TopLevelStatements>( source_location );

  src_processor.use_module( "basicio", source_location );
  src_processor.process_source( *sf );

  return compiler_workspace;
}

}  // namespace Pol::Bscript::Compiler
