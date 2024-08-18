/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_COMPILERCFG_H
#define BSCRIPT_COMPILERCFG_H

#include <string>
#include <vector>

namespace Pol
{
namespace Bscript
{
struct CompilerConfig
{
  std::vector<std::string> PackageRoot;
  std::string IncludeDirectory;
  std::string ModuleDirectory;
  std::string PolScriptRoot;
  bool GenerateListing;
  bool GenerateDebugInfo;
  bool GenerateDebugTextInfo;
  int VerbosityLevel;
  bool DisplayWarnings;
  bool GenerateDependencyInfo;
  bool CompileAspPages;
  bool AutoCompileByDefault;
  bool UpdateOnlyOnAutoCompile;
  bool OnlyCompileUpdatedScripts;
  bool WatchModeByDefault;
  bool DisplaySummary;
  bool DisplayUpToDateScripts;
  bool OptimizeObjectMembers;
  bool ErrorOnWarning;
  bool ThreadedCompilation;
  int NumberOfThreads;
  bool ParanoiaWarnings;
  bool ErrorOnFileCaseMissmatch;
  int EmParseTreeCacheSize;
  int IncParseTreeCacheSize;

  // Formatter
  size_t FormatterLineWidth = 80;
  bool FormatterKeepKeywords = false;
  unsigned short FormatterIndentLevel = 2;
  bool FormatterMergeEmptyLines = true;
  bool FormatterEmptyParenthesisSpacing = false;
  bool FormatterEmptyBracketSpacing = false;
  bool FormatterConditionalParenthesisSpacing = true;
  bool FormatterParenthesisSpacing = true;
  bool FormatterBracketSpacing = true;
  bool FormatterDelimiterSpacing = true;
  bool FormatterAssignmentSpacing = true;
  bool FormatterComparisonSpacing = true;
  bool FormatterOperatorSpacing = true;
  bool FormatterWindowsLineEndings = false;
  bool FormatterUseTabs = false;
  unsigned short FormatterTabWidth = 4;
  bool FormatterInsertNewlineAtEOF = false;
  bool FormatterFormatInsideComments = true;

  void Read( const std::string& path );
  void SetDefaults();
};

extern CompilerConfig compilercfg;
}  // namespace Bscript
}  // namespace Pol
#endif
