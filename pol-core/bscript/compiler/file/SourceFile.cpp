#include "SourceFile.h"

#include "clib/filecont.h"
#include "clib/fileutil.h"
#include "clib/strutil.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "compilercfg.h"

using EscriptGrammar::EscriptLexer;
using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Legacy
{
bool is_web_script( const char* filename );
std::string preprocess_web_script( const std::string& input );
}

namespace Pol::Bscript::Compiler
{
SourceFile::SourceFile( const std::string& pathname, const std::string& contents, Profile& profile )
    : pathname( pathname ),
      input( contents ),
      conformer( &input ),
      lexer( &conformer ),
      tokens( &lexer ),
      parser( &tokens ),
      error_listener( pathname, profile ),
      compilation_unit( nullptr ),
      module_unit( nullptr ),
      access_count( 0 )
{
  input.name = pathname;

  lexer.removeErrorListeners();
  lexer.addErrorListener( &error_listener );
  parser.removeErrorListeners();
  parser.addErrorListener( &error_listener );
}

SourceFile::~SourceFile() = default;

void SourceFile::propagate_errors_to( Report& report, const SourceFileIdentifier& ident )
{
  error_listener.propagate_errors_to( report, ident );
}

#if defined(_WIN32) || defined(__APPLE__)
bool SourceFile::enforced_case_sensitivity_mismatch( const SourceLocation& referencing_location,
                                                     const std::string& pathname, Report& report )
{
  std::string truename = Clib::GetTrueName( pathname.c_str() );
  std::string filepart = Clib::GetFilePart( pathname.c_str() );
  if ( truename != filepart && Clib::FileExists( pathname ) )
  {
    if ( compilercfg.ErrorOnFileCaseMissmatch )
    {
      report.error( referencing_location, "Case mismatch: \n", "  Specified:  ", filepart, "\n",
                    "  Filesystem: ", truename, "\n" );
      return true;
    }

    report.warning( referencing_location, "Case mismatch: \n", "  Specified:  ", filepart, "\n",
                    "  Filesystem: ", truename, "\n" );
  }
  return false;
}
#else
bool SourceFile::enforced_case_sensitivity_mismatch( const SourceLocation&, const std::string&,
                                                     Report& )
{
  return false;
}
#endif

std::shared_ptr<SourceFile> SourceFile::load( const SourceFileIdentifier& ident, Profile& profile,
                                              Report& report )
{
  const std::string& pathname = ident.pathname;
  try
  {
    Clib::FileContents fc( pathname.c_str(), true );
    std::string contents( fc.contents() );

    Clib::sanitizeUnicodeWithIso( &contents );

    if ( Legacy::is_web_script( pathname.c_str() ) )
    {
      contents = Legacy::preprocess_web_script( contents );
    }

    return std::make_shared<SourceFile>( pathname, contents, profile );
  }
  catch ( ... )
  {
    report.error( ident, "Unable to read file '", pathname, "'.\n" );
    return {};
  }
}

EscriptGrammar::EscriptParser::CompilationUnitContext* SourceFile::get_compilation_unit(
    Report& report, const SourceFileIdentifier& ident )
{
  if ( !compilation_unit )
  {
    std::lock_guard<std::mutex> guard( mutex );
    if ( !compilation_unit )
      compilation_unit = parser.compilationUnit();
  }
  ++access_count;
  propagate_errors_to( report, ident );
  return compilation_unit;
}

EscriptGrammar::EscriptParser::ModuleUnitContext* SourceFile::get_module_unit(
    Report& report, const SourceFileIdentifier& ident )
{
  if ( !module_unit )
  {
    std::lock_guard<std::mutex> guard( mutex );
    if ( !module_unit )
      module_unit = parser.moduleUnit();
  }
  ++access_count;
  propagate_errors_to( report, ident );
  return module_unit;
}

}  // namespace Pol::Bscript::Compiler
