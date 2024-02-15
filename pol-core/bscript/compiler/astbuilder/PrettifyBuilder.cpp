#include "PrettifyBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/astbuilder/PrettifyFileProcessor.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/JsonAst.h"
#include "clib/timer.h"
#include <iterator>

namespace Pol::Bscript::Compiler
{
PrettifyBuilder::PrettifyBuilder( SourceFileLoader& source_loader, Profile& profile,
                                  Report& report )
    : source_loader( source_loader ), profile( profile ), report( report )
{
}

std::string PrettifyBuilder::build( const std::string& pathname, bool is_module )
{
  auto ident = std::make_unique<SourceFileIdentifier>( 0, pathname );

  SourceLocation source_location( ident.get(), 0, 0 );

  auto sf = SourceFile::load( *ident, source_loader, profile, report );

  if ( !sf || report.error_count() )
  {
    report.error( *ident, "Unable to load '{}'.", pathname );
    return {};
  }
  PrettifyFileProcessor prettify_processor( *ident, profile, report );
  if ( is_module )
    prettify_processor.process_module_unit( *sf );
  else
    prettify_processor.process_compilation_unit( *sf );

  if ( report.error_count() )
    return {};
  return prettify_processor.prettify();
}

}  // namespace Pol::Bscript::Compiler
