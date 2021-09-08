#include "CompilerWorkspaceBuilder.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableUserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/ModuleProcessor.h"
#include "bscript/compiler/astbuilder/SourceFileProcessor.h"
#include "bscript/compiler/astbuilder/UserFunctionVisitor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "clib/timer.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspaceBuilder::CompilerWorkspaceBuilder( SourceFileLoader& source_loader,
                                                    SourceFileCache& em_cache,
                                                    SourceFileCache& inc_cache, Profile& profile,
                                                    Report& report )
    : source_loader( source_loader ),
      em_cache( em_cache ),
      inc_cache( inc_cache ),
      profile( profile ),
      report( report )
{
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build(
    const std::string& pathname, UserFunctionInclusion user_function_inclusion,
    bool is_diagnostics_mode )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>( report );
  BuilderWorkspace workspace( *compiler_workspace, em_cache, inc_cache, profile, report,
                              is_diagnostics_mode );

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  {
    report.error( *ident, "Refusing to load '", pathname, "'." );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return {};
  }

  auto sf = SourceFile::load( *ident, source_loader, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '", pathname, "'." );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return {};
  }

  SourceFileProcessor src_processor( *ident, workspace, true, user_function_inclusion );

  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
  workspace.compiler_workspace.source = sf;
  workspace.source_files[sf->pathname] = sf;

  compiler_workspace->top_level_statements =
      std::make_unique<TopLevelStatements>( source_location );

  src_processor.use_module( "basic", source_location );
  src_processor.use_module( "basicio", source_location );
  src_processor.process_source( *sf );

  if ( report.error_count() == 0 )
    build_referenced_user_functions( workspace );

  return compiler_workspace;
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build_module(
    const std::string& pathname, bool is_diagnostics_mode )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>( report );
  BuilderWorkspace workspace( *compiler_workspace, em_cache, inc_cache, profile, report,
                              is_diagnostics_mode );

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  {
    report.error( *ident, "Refusing to load '", pathname, "'." );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return {};
  }

  auto sf = em_cache.load( *ident, report );
  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '", pathname, "'." );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return {};
  }

  workspace.source_files[sf->pathname] = sf;
  compiler_workspace->source = sf;
  compiler_workspace->top_level_statements =
      std::make_unique<TopLevelStatements>( source_location );

  std::string module_name = pathname.substr( 0, pathname.find_last_of( "." ) );

  ModuleProcessor module_processor( *ident, workspace, module_name );

  Pol::Tools::HighPerfTimer get_module_unit_timer;
  auto module_unit_context = sf->get_module_unit( report, *ident );
  long long parse_elapsed = get_module_unit_timer.ellapsed().count();
  profile.parse_em_micros += parse_elapsed;
  profile.parse_em_count++;

  Pol::Tools::HighPerfTimer visit_module_unit_timer;
  module_unit_context->accept( &module_processor );
  long long ast_elapsed = visit_module_unit_timer.ellapsed().count();
  profile.ast_em_micros.fetch_add( ast_elapsed );

  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );

  return compiler_workspace;
}


void CompilerWorkspaceBuilder::build_referenced_user_functions( BuilderWorkspace& workspace )
{
  Pol::Tools::HighPerfTimer timer;

  std::vector<AvailableUserFunction> to_build;
  while ( workspace.function_resolver.resolve( to_build ) )
  {
    for ( auto& auf : to_build )
    {
      UserFunctionVisitor user_function_visitor( *auf.source_location.source_file_identifier,
                                                 workspace );

      auf.parse_rule_context->accept( &user_function_visitor );
    }
    to_build.clear();
  }

  workspace.profile.ast_resolve_functions_micros += timer.ellapsed().count();
}

}  // namespace Pol::Bscript::Compiler
