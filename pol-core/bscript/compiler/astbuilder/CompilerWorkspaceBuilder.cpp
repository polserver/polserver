#include "CompilerWorkspaceBuilder.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/GeneratedFunction.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/GeneratedFunctionBuilder.h"
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
                                                    SourceFileCache& inc_cache,
                                                    bool continue_on_error, Profile& profile,
                                                    Report& report )
    : source_loader( source_loader ),
      em_cache( em_cache ),
      inc_cache( inc_cache ),
      continue_on_error( continue_on_error ),
      profile( profile ),
      report( report )
{
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build(
    const std::string& pathname, UserFunctionInclusion user_function_inclusion )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>( report, em_cache, inc_cache,
                                                                 continue_on_error, profile );
  auto& workspace = compiler_workspace->builder_workspace;

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  {
    report.error( *ident, "Refusing to load '{}'.", pathname );
    workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident ) );
    return {};
  }

  auto sf = SourceFile::load( *ident, source_loader, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '{}'.", pathname );
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

  if ( continue_on_error || report.error_count() == 0 )
    build_referenced_user_functions( workspace );

  return compiler_workspace;
}

std::unique_ptr<CompilerWorkspace> CompilerWorkspaceBuilder::build_module(
    const std::string& pathname )
{
  auto compiler_workspace = std::make_unique<CompilerWorkspace>( report, em_cache, inc_cache,
                                                                 continue_on_error, profile );
  auto& workspace = compiler_workspace->builder_workspace;

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

  std::vector<std::unique_ptr<AvailableSecondPassTarget>> to_build;
  std::vector<std::unique_ptr<GeneratedFunction>> generated_functions;

  int resolves_done = 0;
  while ( workspace.function_resolver.resolve( to_build ) )
  {
    Pol::Tools::HighPerfTimer resolve_timer;
    ++resolves_done;
    report.debug( *workspace.compiler_workspace.top_level_statements, "Resolution {} starting.",
                  resolves_done );

    for ( auto& target : to_build )
    {
      report.debug( *workspace.compiler_workspace.top_level_statements, "Resolving {}", *target );
      if ( target->type == AvailableSecondPassTarget::Type::ParseTree )
      {
        auto apt = static_cast<AvailableParseTree*>( target.get() );
        UserFunctionVisitor user_function_visitor( *apt->source_location.source_file_identifier,
                                                   workspace, apt->scope.string(), apt->context );

        apt->parse_rule_context->accept( &user_function_visitor );
      }
      else if ( target->type == AvailableSecondPassTarget::Type::GeneratedFunction )
      {
        auto agf = static_cast<AvailableGeneratedFunction*>( target.get() );
        auto cd = static_cast<ClassDeclaration*>( agf->context );
        auto name = agf->type == UserFunctionType::Super ? "super" : cd->name;

        auto super =
            std::make_unique<GeneratedFunction>( cd->source_location, cd, agf->type, name );
        workspace.function_resolver.register_user_function( cd->name, super.get() );

        // We have to generate the constructor functions first, so that super()
        // can properly call them. This ordering mechanism is an artifact of the
        // way the generated functions are scheduled to build by
        // FunctionResolver.
        //
        // The UserFunctionBuilder registers super() and ctor generated
        // functions as _available_ without any logic. If there was better logic
        // at registration (eg. when a base ClassDeclaration is registered,
        // register children classes' super() as available), this ordering here
        // would not be needed.
        if ( agf->type == UserFunctionType::Super )
          generated_functions.push_back( std::move( super ) );
        else
          generated_functions.insert( generated_functions.begin(), std::move( super ) );
      }
    }
    report.debug( *workspace.compiler_workspace.top_level_statements,
                  "Resolution {} complete with {} parse trees in {} micros.", resolves_done,
                  to_build.size(), resolve_timer.ellapsed().count() );
    to_build.clear();
  };

  // We must build the generated functions _after_ the AST resolution: generated
  // functions reference class links.
  for ( auto& function : generated_functions )
  {
    GeneratedFunctionBuilder tree_builder( *function->source_location.source_file_identifier,
                                           workspace );

    if ( function->type == UserFunctionType::Super )
      tree_builder.super_function( function );
    else if ( function->type == UserFunctionType::Constructor )
      tree_builder.constructor_function( function );
    else
      function->internal_error( "unknown UserFunctionType" );

    report.debug( *workspace.compiler_workspace.top_level_statements,
                  "Generated function {} takes {} params", function->name,
                  function->parameter_count() );

    workspace.compiler_workspace.user_functions.push_back( std::move( function ) );
  }

  workspace.profile.ast_resolve_functions_micros += timer.ellapsed().count();
}

}  // namespace Pol::Bscript::Compiler
