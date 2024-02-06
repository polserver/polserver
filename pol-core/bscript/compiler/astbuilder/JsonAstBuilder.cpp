#include "JsonAstBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/astbuilder/JsonAstFileProcessor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/JsonAst.h"
#include "clib/timer.h"

namespace Pol::Bscript::Compiler
{
JsonAstBuilder::JsonAstBuilder( SourceFileLoader& source_loader, Profile& profile, Report& report )
    : source_loader( source_loader ), profile( profile ), report( report )
{
}

std::string JsonAstBuilder::build( const std::string& pathname, bool is_module )
{
  // auto compiler_workspace = std::make_unique<CompilerWorkspace>( report, em_cache, inc_cache,
  //                                                                continue_on_error, profile );
  // auto& workspace = compiler_workspace->builder_workspace;

  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  // if ( SourceFile::enforced_case_sensitivity_mismatch( source_location, pathname, report ) )
  // {
  //   report.error( *ident, "Refusing to load '{}'.", pathname );
  //   workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident )
  //   ); return {};
  // }

  Tools::Timer<Tools::DebugT> timerload( "Sourcefile::load" );
  auto sf = SourceFile::load( *ident, source_loader, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '{}'.", pathname );
    return {};
  }
  timerload.stop();

  // auto json_ast = std::make_unique<JsonAst>();

  JsonAstFileProcessor json_ast_processor( *ident, profile, report );

  Tools::Timer<Tools::DebugT> timerproc( "process" );
  auto json_ast_any = is_module ? json_ast_processor.process_module_unit( *sf )
                                : json_ast_processor.process_compilation_unit( *sf );

  auto* json_ast = std::any_cast<picojson::value>( &json_ast_any );
  timerproc.stop();
  Tools::Timer<Tools::DebugT> timers( "serialize" );
  auto res = json_ast->serialize();
  timers.stop();
  return res;
  // SourceFileProcessor src_processor( *ident, workspace, true, user_function_inclusion );

  // workspace.compiler_workspace.referenced_source_file_identifiers.push_back( std::move( ident )
  // ); workspace.compiler_workspace.source = sf; workspace.source_files[sf->pathname] = sf;

  // compiler_workspace->top_level_statements =
  //     std::make_unique<TopLevelStatements>( source_location );

  // src_processor.use_module( "basic", source_location );
  // src_processor.use_module( "basicio", source_location );
  // src_processor.process_source( *sf );

  // if ( continue_on_error || report.error_count() == 0 )
  //   build_referenced_user_functions( workspace );

  // return compiler_workspace;
}

}  // namespace Pol::Bscript::Compiler
