#include "CompilerWorkspaceBuilder.h"

#include "clib/timer.h"
#include "bscript/compiler/LegacyFunctionOrder.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableUserFunction.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/SourceFileProcessor.h"
#include "bscript/compiler/astbuilder/UserFunctionVisitor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspaceBuilder::CompilerWorkspaceBuilder( SourceFileCache& em_cache,
                                                    SourceFileCache& inc_cache,
                                                    Profile& profile, Report& report )
    : em_cache( em_cache ), inc_cache( inc_cache ), profile( profile ), report( report )
{
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build(
    const std::string& pathname, const LegacyFunctionOrder* legacy_function_order,
    UserFunctionInclusion user_function_inclusion )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>( report );
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

  SourceFileProcessor src_processor( *ident, workspace, true, user_function_inclusion );

  workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
  workspace.source_files[sf->pathname] = sf;

  compiler_workspace->top_level_statements =
      std::make_unique<TopLevelStatements>( source_location );

  src_processor.use_module( "basic", source_location );
  src_processor.use_module( "basicio", source_location );
  src_processor.process_source( *sf );

  if ( report.error_count() == 0 )
    build_referenced_user_functions( workspace );

  if ( legacy_function_order )
  {
    compiler_workspace->module_functions_in_legacy_order =
        get_module_functions_in_order( workspace, *legacy_function_order );
  }

  return compiler_workspace;
}

std::vector<const ModuleFunctionDeclaration*>
CompilerWorkspaceBuilder::get_module_functions_in_order( BuilderWorkspace& workspace,
                                                         const LegacyFunctionOrder& h )
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
