#include "JsonAstBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/astbuilder/JsonAstFileProcessor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/JsonAst.h"
#include "clib/timer.h"
#include <iterator>

namespace Pol::Bscript::Compiler
{
JsonAstBuilder::JsonAstBuilder( SourceFileLoader& source_loader, Profile& profile, Report& report )
    : source_loader( source_loader ), profile( profile ), report( report )
{
}

std::string JsonAstBuilder::build( const std::string& pathname, bool is_module )
{
  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  auto sf = SourceFile::load( *ident, source_loader, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '{}'.", pathname );
    return {};
  }
  JsonAstFileProcessor json_ast_processor( *ident, profile, report );
  auto json_ast_any = is_module ? json_ast_processor.process_module_unit( *sf )
                                : json_ast_processor.process_compilation_unit( *sf );

  auto* json_ast = std::any_cast<picojson::value>( &json_ast_any );
  auto res = json_ast->serialize( true );
  INFO_PRINTLN( res );
  //  auto pretty = prettify( *json_ast, comments );
  return json_ast_processor.pretty();
}

}  // namespace Pol::Bscript::Compiler
