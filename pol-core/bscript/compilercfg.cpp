/** @file
 *
 * @par History
 */


#include "compilercfg.h"

#include <stdlib.h>

#include "../clib/Program/ProgramConfig.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"

namespace Pol
{
namespace Bscript
{
void CompilerConfig::Read( const std::string& path )
{
#ifdef _WIN32
  bool win_platform = true;
#else
  bool win_platform = false;
#endif

  Clib::ConfigFile cf( path.c_str() );
  Clib::ConfigElem elem;
  cf.readraw( elem );

  PackageRoot.clear();
  IncludeDirectory.clear();

  std::string tmp;
  while ( elem.remove_prop( "PackageRoot", &tmp ) )
  {
    PackageRoot.push_back( Clib::normalized_dir_form( tmp ) );
  }
  if ( elem.remove_prop( "IncludeDirectory", &tmp ) )
  {
    IncludeDirectory = Clib::normalized_dir_form( tmp );
  }
  ModuleDirectory = Clib::normalized_dir_form( elem.remove_string( "ModuleDirectory" ) );
  PolScriptRoot = Clib::normalized_dir_form( elem.remove_string( "PolScriptRoot" ) );
  GenerateListing = elem.remove_bool( "GenerateListing", false );
  GenerateDebugInfo = elem.remove_bool( "GenerateDebugInfo", false );
  GenerateDebugTextInfo = elem.remove_bool( "GenerateDebugTextInfo", false );

  VerbosityLevel = elem.remove_int( "VerbosityLevel", 0 );
  DisplayWarnings = elem.remove_bool( "DisplayWarnings", false );
  CompileAspPages = elem.remove_bool( "CompileAspPages", false );
  AutoCompileByDefault = elem.remove_bool( "AutoCompileByDefault", false );
  UpdateOnlyOnAutoCompile = elem.remove_bool( "UpdateOnlyOnAutoCompile", false );
  OnlyCompileUpdatedScripts = elem.remove_bool( "OnlyCompileUpdatedScripts", false );
  WatchModeByDefault = elem.remove_bool( "WatchModeByDefault", false );
  DisplaySummary = elem.remove_bool( "DisplaySummary", false );
  OptimizeObjectMembers = elem.remove_bool( "OptimizeObjectMembers", true );
  ErrorOnWarning = elem.remove_bool( "ErrorOnWarning", false );
  GenerateDependencyInfo = elem.remove_bool( "GenerateDependencyInfo", OnlyCompileUpdatedScripts );

  DisplayUpToDateScripts = elem.remove_bool( "DisplayUpToDateScripts", true );
  ThreadedCompilation = elem.remove_bool( "ThreadedCompilation", false );
  NumberOfThreads = elem.remove_int( "NumberOfThreads", 0 );
  ParanoiaWarnings = elem.remove_bool( "ParanoiaWarnings", false );
  ErrorOnFileCaseMissmatch = elem.remove_bool( "ErrorOnFileCaseMissmatch", false );

  EmParseTreeCacheSize = elem.remove_int( "EmParseTreeCacheSize", 25 );
  IncParseTreeCacheSize = elem.remove_int( "IncParseTreeCacheSize", 50 );


  FormatterLineWidth = elem.remove_unsigned( "FormatterLineWidth", 100 );
  FormatterKeepKeywords = elem.remove_bool( "FormatterKeepKeywords", false );
  FormatterIndentLevel = elem.remove_ushort( "FormatterIndentLevel", 2 );
  FormatterMergeEmptyLines = elem.remove_bool( "FormatterMergeEmptyLines", true );
  FormatterEmptyParenthesisSpacing = elem.remove_bool( "FormatterEmptyParenthesisSpacing", false );
  FormatterEmptyBracketSpacing = elem.remove_bool( "FormatterEmptyBracketSpacing", false );
  FormatterConditionalParenthesisSpacing =
      elem.remove_bool( "FormatterConditionalParenthesisSpacing", true );
  FormatterParenthesisSpacing = elem.remove_bool( "FormatterParenthesisSpacing", true );
  FormatterBracketSpacing = elem.remove_bool( "FormatterBracketSpacing", true );
  FormatterDelimiterSpacing = elem.remove_bool( "FormatterDelimiterSpacing", true );
  FormatterAssignmentSpacing = elem.remove_bool( "FormatterAssignmentSpacing", true );
  FormatterComparisonSpacing = elem.remove_bool( "FormatterComparisonSpacing", true );
  FormatterOperatorSpacing = elem.remove_bool( "FormatterOperatorSpacing", true );
  FormatterWindowsLineEndings = elem.remove_bool( "FormatterWindowsLineEndings", win_platform );
  FormatterUseTabs = elem.remove_bool( "FormatterUseTabs", false );
  FormatterTabWidth = elem.remove_ushort( "FormatterTabWidth", 4 );
  FormatterInsertNewlineAtEOF = elem.remove_bool( "FormatterInsertNewlineAtEOF", true );
  FormatterFormatInsideComments = elem.remove_bool( "FormatterFormatInsideComments", true );
  FormatterBracketAttachToType = elem.remove_bool( "FormatterBracketAttachToType", true );
  FormatterAlignTrailingComments = elem.remove_bool( "FormatterAlignTrailingComments", true );
  FormatterAlignConsecutiveShortCaseStatements =
      elem.remove_bool( "FormatterAlignConsecutiveShortCaseStatements", true );
  FormatterAllowShortCaseLabelsOnASingleLine =
      elem.remove_bool( "FormatterAllowShortCaseLabelsOnASingleLine", true );
  FormatterAllowShortFuncRefsOnASingleLine =
      elem.remove_bool( "FormatterAllowShortFuncRefsOnASingleLine", true );


  // This is where we TRY to validate full paths from what was provided in the
  // ecompile.cfg. Maybe Turley or Shini can find the best way to do this in *nix.
#ifdef WIN32
  std::string MyPath = path.c_str();
  // If it's just "ecompile.cfg", let's change it to the exe's path which it SHOULD be
  // with.
  if ( stricmp( MyPath.c_str(), "ecompile.cfg" ) == 0 )
  {
    std::string workingDir = PROG_CONFIG::programDir();

    // Let's find the NEXT-TO-LAST / in the path, and remove from there on. Oh yay!
    // To bad we can't just force everyone to use ABSOLUTE PATHS NANDO. :o
    MyPath = workingDir.substr( 0, workingDir.length() - 1 );
    MyPath = MyPath.substr( 0, MyPath.find_last_of( '/' ) + 1 );
  }
  if ( IncludeDirectory.find( ':' ) == std::string::npos )
  {
    if ( IncludeDirectory.substr( 0, 1 ) !=
         "." )  // Let's make sure they didn't try using this method
    {
      IncludeDirectory = MyPath + IncludeDirectory;
    }
  }
  if ( ModuleDirectory.find( ':' ) == std::string::npos )
  {
    if ( ModuleDirectory.substr( 0, 1 ) !=
         "." )  // Let's make sure they didn't try using this method
    {
      ModuleDirectory = MyPath + ModuleDirectory;
    }
  }
  if ( PolScriptRoot.find( ':' ) == std::string::npos )
  {
    if ( PolScriptRoot.substr( 0, 1 ) != "." )  // Let's make sure they didn't try using this method
    {
      PolScriptRoot = MyPath + PolScriptRoot;
    }
  }
  for ( unsigned pr = 0; pr < PackageRoot.size(); ++pr )
  {
    if ( PackageRoot[pr].find( ':' ) == std::string::npos )
    {
      if ( PackageRoot[pr].substr( 0, 1 ) !=
           "." )  // Let's make sure they didn't try using this method
      {
        PackageRoot[pr] = MyPath + PackageRoot[pr];
      }
    }
  }

#endif
}

void CompilerConfig::SetDefaults()
{
  const char* tmp;

  tmp = getenv( "ECOMPILE_PATH_EM" );
  ModuleDirectory = tmp ? Clib::normalized_dir_form( tmp ) : PROG_CONFIG::programDir();

  tmp = getenv( "ECOMPILE_PATH_INC" );
  IncludeDirectory = tmp ? Clib::normalized_dir_form( tmp ) : PROG_CONFIG::programDir();

  PolScriptRoot = IncludeDirectory;

  DisplayUpToDateScripts = true;

  EmParseTreeCacheSize = 25;
  IncParseTreeCacheSize = 50;
}

CompilerConfig compilercfg;
}  // namespace Bscript
}  // namespace Pol
