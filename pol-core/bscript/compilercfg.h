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
  bool DisplayWarnings;
  bool GenerateDependencyInfo;
  bool CompileAspPages;
  bool AutoCompileByDefault;
  bool UpdateOnlyOnAutoCompile;
  bool OnlyCompileUpdatedScripts;
  bool DisplaySummary;
  bool DisplayUpToDateScripts;
  bool ErrorOnWarning;
  bool ThreadedCompilation;
  int NumberOfThreads;
  bool ParanoiaWarnings;
  bool ErrorOnFileCaseMissmatch;

  void Read( const std::string& path );
  void SetDefaults();
};

extern CompilerConfig compilercfg;
}  // namespace Bscript
}  // namespace Pol
#endif
