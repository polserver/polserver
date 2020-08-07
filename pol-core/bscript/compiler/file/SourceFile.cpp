#include "SourceFile.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"

#include "compiler/Report.h"
#include "compiler/file/SourceFileIdentifier.h"
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
      unambiguous_compilation_unit( nullptr ),
      unambiguous_module_unit( nullptr ),
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

#ifdef _WIN32
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
  std::ifstream ifs( pathname );
  if ( !ifs.is_open() )
  {
    report.error( ident, "Unable to open file '", pathname, "'.\n" );
    return {};
  }
  // INFO_PRINT << "(out)opened " << pathname << "\n";
  // ERROR_PRINT << "(err)opened " << pathname << "\n";

  std::string contents( ( std::istreambuf_iterator<char>( ifs ) ),
                        std::istreambuf_iterator<char>() );

  if ( ifs.fail() )
  {
    report.error( ident, "Unable to read file '", pathname, "'.\n" );
    return {};
  }

  Clib::sanitizeUnicodeWithIso( &contents );

  if ( Legacy::is_web_script( pathname.c_str() ) )
  {
    contents = Legacy::preprocess_web_script( contents );
  }

  return std::make_shared<SourceFile>( pathname, contents, profile );
}

antlr4::ParserRuleContext* SourceFile::get_compilation_unit( Report& report,
                                                             const SourceFileIdentifier& ident )
{
  if ( compilercfg.UseUnambiguousGrammar )
  {
    return get_unambiguous_compilation_unit( report, ident );
  }
  else
  {
    return get_ambiguous_compilation_unit( report, ident );
  }
}

antlr4::ParserRuleContext* SourceFile::get_module_unit( Report& report,
                                                        const SourceFileIdentifier& ident )
{
  if ( compilercfg.UseUnambiguousGrammar )
  {
    return get_unambiguous_module_unit( report, ident );
  }
  else
  {
    return get_ambiguous_module_unit( report, ident );
  }
}

EscriptGrammar::EscriptParser::CompilationUnitContext* SourceFile::get_ambiguous_compilation_unit(
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

EscriptGrammar::EscriptParser::ModuleUnitContext* SourceFile::get_ambiguous_module_unit(
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

EscriptGrammar::EscriptParser::UnambiguousCompilationUnitContext*
SourceFile::get_unambiguous_compilation_unit( Report& report, const SourceFileIdentifier& ident )
{
  if ( !unambiguous_compilation_unit )
  {
    std::lock_guard<std::mutex> guard( mutex );
    if ( !unambiguous_compilation_unit )
      unambiguous_compilation_unit = parser.unambiguousCompilationUnit();
  }
  ++access_count;
  propagate_errors_to( report, ident );
  return unambiguous_compilation_unit;
}

EscriptGrammar::EscriptParser::UnambiguousModuleUnitContext*
SourceFile::get_unambiguous_module_unit( Report& report, const SourceFileIdentifier& ident )
{
  if ( !unambiguous_module_unit )
  {
    std::lock_guard<std::mutex> guard( mutex );
    if ( !unambiguous_module_unit )
      unambiguous_module_unit = parser.unambiguousModuleUnit();
  }
  ++access_count;
  propagate_errors_to( report, ident );
  return unambiguous_module_unit;
}

}  // namespace Pol::Bscript::Compiler