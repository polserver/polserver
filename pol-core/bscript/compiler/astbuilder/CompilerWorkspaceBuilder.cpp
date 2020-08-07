#include "CompilerWorkspaceBuilder.h"

#include "BuilderWorkspace.h"
#include "SourceFileProcessor.h"
#include "compiler/LegacyFunctionOrder.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Statement.h"
#include "compiler/ast/TopLevelStatements.h"
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
    const std::string& pathname, const LegacyFunctionOrder* legacy_function_order )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>();
  BuilderWorkspace workspace( *compiler_workspace, em_cache, inc_cache, profile, report );

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  {
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

  src_processor.use_module( "basicio", source_location );
  src_processor.process_source( *sf );

  std::vector<std::unique_ptr<Statement>> empty {};
  compiler_workspace->top_level_statements = std::make_unique<TopLevelStatements>(
      source_location, std::move( empty ) );

  if ( legacy_function_order )
  {
    compiler_workspace->module_functions_in_legacy_order =
        get_module_functions_in_order( workspace, *legacy_function_order );
  }

  return compiler_workspace;
}

std::vector<const ModuleFunctionDeclaration*> CompilerWorkspaceBuilder::get_module_functions_in_order(
    BuilderWorkspace& workspace, const LegacyFunctionOrder& h )
{
  std::vector<const ModuleFunctionDeclaration*> ordered;

  for ( auto& scoped_name : h.modulefunc_emit_order )
  {
    auto func = workspace.function_resolver.find( scoped_name );
    if ( !func )
      throw std::runtime_error( "No modulefunc '" + scoped_name + "' for parity." );

    auto decl = dynamic_cast<const ModuleFunctionDeclaration*>( func );
    if ( !decl )
      throw std::runtime_error( "No ModuleFunctionDeclaration for '" + scoped_name +
                                "' for parity." );

    ordered.push_back( decl );
  }
  return ordered;
}

}  // namespace Pol::Bscript::Compiler
