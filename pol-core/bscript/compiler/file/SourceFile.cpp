#include "SourceFile.h"

#include <cstring>

#include "clib/filecont.h"
#include "clib/fileutil.h"
#include "clib/strutil.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/model/SemanticTokens.h"
#include "compilercfg.h"
#include <EscriptGrammar/EscriptParserVisitor.h>

using EscriptGrammar::EscriptLexer;
using EscriptGrammar::EscriptParser;
using EscriptGrammar::EscriptParserVisitor;

namespace Pol::Bscript::Compiler
{
bool is_web_script( const char* filename );
std::string preprocess_web_script( const std::string& input );

SourceFile::SourceFile( const std::string& pathname, const std::string& contents, Profile& profile )
    : pathname( pathname ),
      input( contents ),
      conformer( &input ),
      lexer( &conformer ),
      token_stream( &lexer ),
      parser( &token_stream ),
      error_listener( pathname, profile ),
      compilation_unit( nullptr ),
      module_unit( nullptr ),
      evaluate_unit( nullptr ),
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

#if defined( _WIN32 ) || defined( __APPLE__ )
bool SourceFile::enforced_case_sensitivity_mismatch( const SourceLocation& referencing_location,
                                                     const std::string& pathname, Report& report )
{
  std::string truename = Clib::GetTrueName( pathname.c_str() );
  std::string filepart = Clib::GetFilePart( pathname.c_str() );
  if ( truename != filepart && Clib::FileExists( pathname ) )
  {
    if ( compilercfg.ErrorOnFileCaseMissmatch )
    {
      report.error( referencing_location,
                    "Case mismatch: \n"
                    "  Specified:  {}\n"
                    "  Filesystem: {}",
                    filepart, truename );
      return true;
    }

    report.warning( referencing_location,
                    "Case mismatch: \n"
                    "  Specified:  {}\n"
                    "  Filesystem: {}",
                    filepart, truename );
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

std::shared_ptr<SourceFile> SourceFile::load( const SourceFileIdentifier& ident,
                                              const SourceFileLoader& source_loader,
                                              Profile& profile, Report& report )
{
  const std::string& pathname = ident.pathname;
  try
  {
    auto contents = source_loader.get_contents( pathname );
    Clib::sanitizeUnicodeWithIso( &contents );

    if ( is_web_script( pathname.c_str() ) )
    {
      contents = preprocess_web_script( contents );
    }

    return std::make_shared<SourceFile>( pathname, contents, profile );
  }
  catch ( ... )
  {
    report.error( ident, "Unable to read file '{}'.", pathname );
    return {};
  }
}

void SourceFile::accept( EscriptParserVisitor& visitor )
{
  antlr4::ParserRuleContext* unit = nullptr;

  if ( ( unit = compilation_unit ) )
  {
    visitor.visit( unit );
  }
  else if ( ( unit = module_unit ) )
  {
    visitor.visit( unit );
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

EscriptGrammar::EscriptParser::EvaluateUnitContext* SourceFile::get_evaluate_unit(
    Report& report )
{
  if ( !evaluate_unit )
  {
    std::lock_guard<std::mutex> guard( mutex );
    if ( !evaluate_unit )
      evaluate_unit = parser.evaluateUnit();
  }
  ++access_count;
  propagate_errors_to( report, SourceFileIdentifier( 0, "<eval>" ) );
  return evaluate_unit;
}

std::unique_ptr<antlr4::Token> SourceFile::get_token_at( const Position& position )
{
  lexer.reset();
  auto tokens = lexer.getAllTokens();
  auto result =
      std::find_if( tokens.begin(), tokens.end(),
                    [&]( const auto& token )
                    {
                      return token->getLine() == position.line_number &&
                             token->getCharPositionInLine() + 1 <= position.character_column &&
                             token->getCharPositionInLine() + 1 + token->getText().length() >=
                                 position.character_column;
                    } );
  if ( result != tokens.end() )
  {
    return std::move( *result );
  }
  return {};
}

SemanticTokens SourceFile::get_tokens()
{
  SemanticTokens tokens;
  lexer.reset();
  for ( const auto& lexer_token : lexer.getAllTokens() )
  {
    auto semantic_token = SemanticToken::from_lexer_token( *lexer_token );
    if ( semantic_token )
    {
      auto& t = *semantic_token;
      tokens.insert( tokens.end(), std::make_move_iterator( t.begin() ),
                     std::make_move_iterator( t.end() ) );
    }
  }
  return tokens;
}

/**
 * Given a file name, tells if this is a web script
 */
bool is_web_script( const char* file )
{
  const char* ext = strstr( file, ".hsr" );
  if ( ext && memcmp( ext, ".hsr", 5 ) == 0 )
    return true;
  ext = strstr( file, ".asp" );
  if ( ext && memcmp( ext, ".asp", 5 ) == 0 )
    return true;
  return false;
}

/**
 * Transforms the raw html page into a script with a single WriteHtml() instruction
 */
std::string preprocess_web_script( const std::string& input )
{
  std::string output;
  output = "use http;";
  output += '\n';

  bool reading_html = true;
  bool source_is_emit = false;
  const char* s = input.c_str();
  std::string acc;
  while ( *s )
  {
    if ( reading_html )
    {
      if ( s[0] == '<' && s[1] == '%' )
      {
        reading_html = false;
        if ( !acc.empty() )
        {
          output += "WriteHtmlRaw( \"" + acc + "\");\n";
          acc = "";
        }
        s += 2;
        source_is_emit = ( s[0] == '=' );
        if ( source_is_emit )
        {
          output += "WriteHtmlRaw( ";
          ++s;
        }
      }
      else
      {
        if ( *s == '\"' )
          acc += "\\\"";
        else if ( *s == '\r' )
          ;
        else if ( *s == '\n' )
          acc += "\\n";
        else
          acc += *s;
        ++s;
      }
    }
    else
    {
      if ( s[0] == '%' && s[1] == '>' )
      {
        reading_html = true;
        s += 2;
        if ( source_is_emit )
          output += " );\n";
      }
      else
      {
        output += *s++;
      }
    }
  }
  if ( !acc.empty() )
    output += "WriteHtmlRaw( \"" + acc + "\");\n";
  return output;
}

}  // namespace Pol::Bscript::Compiler
